//
// Created by y123456 on 2021/10/10.
//

#include "VulkanRender.h"
#include "RenderState.h"
#include "../backend/Vulkan/VulkanSwapChain.h"
#include "ApplicationResource.h"
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
using namespace render::backend;

VulkanRender::VulkanRender(render::backend::Driver *driver,
                           VkExtent2D size)
    : driver(driver)
{
}

VulkanRender::~VulkanRender()
{
    shutdown();
}

void VulkanRender::init(ApplicationResource *resource)
{
    scene_bind_set = driver->createBindSet();

    std::array<const VulkanTexture *, 8> textures =
    {
        resource->getAlbedoTexture(),
        resource->getNormalTexture(),
        resource->getAOTexture(),
        resource->getShadingTexture(),
        resource->getEmissionTexture(),
        resource->getHDRIEnvironmentubeMap(0),
        resource->getIrridanceCubeMap(0),
        resource->getBakedBRDF(),
    };

    for (int k = 0; k < textures.size(); k++)
        driver->bindTexture(scene_bind_set, k, textures[k]->getTexture());
}

void VulkanRender::shutdown()
{
    driver->destroyBindSet(scene_bind_set);
    scene_bind_set = nullptr;
}

void VulkanRender::update(RenderState &state, ApplicationResource *scene)
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

void VulkanRender::render(ApplicationResource *scene, const VulkanRenderFrame &frame)
{
    const VulkanShader *vertShader = scene->getPBRVertexShader();
    const VulkanShader *fragShader = scene->getPBRFragmentShader();
    const VulkanShader *skyboxVertexShader = scene->getSkyboxVertexShader();
    const VulkanShader *skyboxFragmentShader = scene->getSkyboxFragmentShader();

    driver->clearBindSets();
    driver->clearShaders();
    driver->pushBindSet(frame.bind_set);
    driver->pushBindSet(scene_bind_set);

    driver->setShader(ShaderType::VERTEX, skyboxVertexShader->getShader());
    driver->setShader(ShaderType::FRAGMENT, skyboxFragmentShader->getShader());
    driver->drawIndexedPrimitive(frame.command_buffer, scene->getSkyboxMesh()->getPrimitive());

    driver->setShader(ShaderType::VERTEX, vertShader->getShader());
    driver->setShader(ShaderType::FRAGMENT, fragShader->getShader());
    driver->drawIndexedPrimitive(frame.command_buffer, scene->getMesh()->getPrimitive());
}

void VulkanRender::resize(const VulkanSwapChain *swapChain)
{
    extent = swapChain->getExtent();
}


void VulkanRender::setEnvironment(const ApplicationResource *resource, uint8_t index)
{
    const VulkanTexture* environment_texture= resource->getHDRIEnvironmentubeMap(index);
    const VulkanTexture* irrandance_texture= resource->getIrridanceCubeMap(index);
    driver->bindTexture(scene_bind_set,5,environment_texture->getTexture());
    driver->bindTexture(scene_bind_set,6,irrandance_texture->getTexture());
}
