//
// Created by y123456 on 2021/10/10.
//

#include "../backend/Vulkan/VulkanSwapChain.h"
#include "ApplicationResource.h"
#include "Render.h"
#include "RenderState.h"
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
using namespace render::backend;

Render::Render(render::backend::Driver *driver)
    : driver(driver)
{
}

Render::~Render()
{
    shutdown();
}

void Render::init(ApplicationResource *resource)
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

void Render::shutdown()
{
    driver->destroyBindSet(scene_bind_set);
    scene_bind_set = nullptr;
}

void Render::render(ApplicationResource *scene, const VulkanRenderFrame &frame)
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


void Render::setEnvironment(const ApplicationResource *resource, uint8_t index)
{
    const VulkanTexture* environment_texture= resource->getHDRIEnvironmentubeMap(index);
    const VulkanTexture* irrandance_texture= resource->getIrridanceCubeMap(index);
    driver->bindTexture(scene_bind_set,5,environment_texture->getTexture());
    driver->bindTexture(scene_bind_set,6,irrandance_texture->getTexture());
}
