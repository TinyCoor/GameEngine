//
// Created by y123456 on 2021/10/10.
//

#include "VulkanRender.h"
#include "RenderState.h"
#include "../backend/Vulkan/VulkanSwapChain.h"
#include "VulkanRenderScene.h"
#include "../backend/Vulkan/VulkanUtils.h"

#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

namespace render::backend::vulkan {
VulkanRender::VulkanRender(render::backend::Driver *driver,
                           VkExtent2D size)
    : driver(driver),
      extent(size),
      hdriToCubeRenderer( driver),
      diffuseIrradianceRenderer( driver),
      environmentCubemap(driver),
      diffuseIrradianceCubemap(driver),
      brdfBaked(driver),
      brdfRender(driver)
{
    context = static_cast<vulkan::VulkanDriver*>(driver)->GetDevice();
}

VulkanRender::~VulkanRender()
{
    shutdown();
}

void VulkanRender::init(VulkanRenderScene *scene)
{
    brdfBaked.create2D(render::backend::Format::R16G16_SFLOAT, 256, 256, 1);
    environmentCubemap.createCube(render::backend::Format::R32G32B32A32_SFLOAT, 256, 256, 1);
    diffuseIrradianceCubemap.createCube(render::backend::Format::R32G32B32A32_SFLOAT, 256, 256, 1);

    brdfRender.init(&brdfBaked);
    brdfRender.render(
        scene->getBakedVertexShader(),
        scene->getBakedFragmentShader()
    );

    hdriToCubeRenderer.init(
        *scene->getCubeVertexShader(),
        *scene->getHDRToCubeFragmentShader(),
        environmentCubemap,
        0
    );

    //TODO unique_ptr
    cubeToPrefilteredRenderers.resize(environmentCubemap.getNumMiplevels() - 1);
    for (int mip = 0; mip < environmentCubemap.getNumMiplevels() - 1; ++mip) {
        VulkanCubeMapRender *mipRenderer = new VulkanCubeMapRender(driver);
        mipRenderer->init(
            *scene->getCubeVertexShader(),
            *scene->getCubeToPrefilteredSpecularShader(),
            environmentCubemap,
            mip + 1);
        cubeToPrefilteredRenderers.emplace_back(mipRenderer);
    }

    diffuseIrradianceRenderer.init(
        *scene->getCubeVertexShader(),
        *scene->getDiffuseToIrridanceShader(),
        diffuseIrradianceCubemap,
        0
    );

    scene_bind_set= driver->createBindSet();

    std::array<VulkanTexture *, 8> textures =
    {
        scene->getAlbedoTexture(),
        scene->getNormalTexture(),
        scene->getAOTexture(),
        scene->getShadingTexture(),
        scene->getEmissionTexture(),
        &environmentCubemap,
        &diffuseIrradianceCubemap,
        &brdfBaked,
    };

    for (int k = 0; k < textures.size(); k++)
        driver->bindTexture(scene_bind_set,k,textures[k]->getTexture());
}

void VulkanRender::shutdown()
{

    hdriToCubeRenderer.shutdown();
    diffuseIrradianceRenderer.shutdown();

    for (int i = 0; i < cubeToPrefilteredRenderers.size(); ++i) {
        cubeToPrefilteredRenderers[i]->shutdown();
        delete cubeToPrefilteredRenderers[i];
    }
    cubeToPrefilteredRenderers.clear();
    brdfRender.shutdown();

    brdfBaked.clearGPUData();
    brdfBaked.clearCPUData();

    environmentCubemap.clearGPUData();
    environmentCubemap.clearCPUData();

    diffuseIrradianceCubemap.clearGPUData();
    diffuseIrradianceCubemap.clearCPUData();

    driver->destroyBindSet(scene_bind_set);
    scene_bind_set = nullptr;

}

void VulkanRender::update(RenderState &state, VulkanRenderScene *scene)
{
    // Render state
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();

    const float rotationSpeed = 0.1f;
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    const glm::vec3 &up = {0.0f, 0.0f, 1.0f};
    const glm::vec3 &zero = {0.0f, 0.0f, 0.0f};

    const float aspect = (float) extent.width / (float) extent.height;
    const float zNear = 0.1f;
    const float zFar = 1000.0f;

    const glm::vec3 &cameraPos = glm::vec3(2.0f, 2.0f, 2.0f);
    const glm::mat4 &rotation = glm::rotate(glm::mat4(1.0f), time * rotationSpeed * glm::radians(90.0f), up);

    state.world = glm::mat4(1.0f);
    state.view = glm::lookAt(cameraPos, zero, up) * rotation;
    state.proj = glm::perspective(glm::radians(60.0f), aspect, zNear, zFar);
    state.proj[1][1] *= -1;
    state.cameraPosWS = glm::vec3(glm::vec4(cameraPos, 1.0f) * rotation);

}

void VulkanRender::render(VulkanRenderScene *scene, const VulkanRenderFrame &frame)
{
    const VulkanShader *vertShader = scene->getPBRVertexShader();
    const VulkanShader *fragShader = scene->getPBRFragmentShader();
    const VulkanShader *skyboxVertexShader = scene->getSkyboxVertexShader();
    const VulkanShader *skyboxFragmentShader = scene->getSkyboxFragmentShader();


    driver->clearBindSets();
    driver->pushBindSet(frame.bind_set);
    driver->pushBindSet(scene_bind_set);
    driver->clearShaders();

    driver->setShader(ShaderType::VERTEX, skyboxVertexShader->getShader());
    driver->setShader(ShaderType::FRAGMENT, skyboxFragmentShader->getShader());
    driver->drawIndexedPrimitive(frame.command_buffer, scene->getSkyboxMesh()->getPrimitive());

    driver->setShader(ShaderType::VERTEX, vertShader->getShader());
    driver->setShader(ShaderType::FRAGMENT, fragShader->getShader());
    driver->drawIndexedPrimitive(frame.command_buffer, scene->getMesh()->getPrimitive());
}

void VulkanRender::setEnvironment(VulkanTexture *texture)
{

    hdriToCubeRenderer.render(*texture);
    //mip
    for (size_t i = 0; i < cubeToPrefilteredRenderers.size(); ++i) {
        float data[4] = {
            static_cast<float>(i) / environmentCubemap.getNumMiplevels(),
            0.0f, 0.0f, 0.0f
        };
        cubeToPrefilteredRenderers[i]->render(environmentCubemap, data, i);
    }

    diffuseIrradianceRenderer.render(environmentCubemap);

    std::array<VulkanTexture *, 2> textures = {
        &environmentCubemap,
        &diffuseIrradianceCubemap,
    };
    driver->bindTexture(scene_bind_set, 5,environmentCubemap.getTexture());
    driver->bindTexture(scene_bind_set, 6,diffuseIrradianceCubemap.getTexture());
}

void VulkanRender::resize(const VulkanSwapChain *swapChain)
{
    extent = swapChain->getExtent();
}

void VulkanRender::reload(VulkanRenderScene *scene)
{
    shutdown();
    init(scene);
}

}
