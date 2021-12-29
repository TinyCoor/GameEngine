//
// Created by y123456 on 2021/10/10.
//

#include "../backend/Vulkan/VulkanSwapChain.h"
#include "ApplicationResource.h"
#include "Render.h"
#include "../backend/Vulkan/SkyLight.h"
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

    std::array<const VulkanTexture *, 5> textures =
    {
        resource->getAlbedoTexture(),
        resource->getNormalTexture(),
        resource->getAOTexture(),
        resource->getShadingTexture(),
        resource->getEmissionTexture(),
    };

    for (int k = 0; k < textures.size(); k++)
        driver->bindTexture(scene_bind_set, k, textures[k]->getTexture());
}

void Render::shutdown()
{
    driver->destroyBindSet(scene_bind_set);
    scene_bind_set = nullptr;
}

void Render::render(const ApplicationResource *scene,const SkyLight *light, const VulkanRenderFrame &frame)
{
    const VulkanShader *pbr_vert_shader = scene->getPBRVertexShader();
    const VulkanShader *pbr_frag_shader = scene->getPBRFragmentShader();
    const VulkanShader *skybox_vertex_shader = scene->getSkyboxVertexShader();
    const VulkanShader *skybox_fragment_shader = scene->getSkyboxFragmentShader();

    driver->clearBindSets();
    driver->clearShaders();
    driver->pushBindSet(frame.bind_set);
    driver->pushBindSet(scene_bind_set);
    driver->pushBindSet(const_cast<render::backend::BindSet *>(light->getBindSet()));

    driver->setShader(ShaderType::VERTEX, skybox_vertex_shader->getShader());
    driver->setShader(ShaderType::FRAGMENT, skybox_fragment_shader->getShader());
    driver->drawIndexedPrimitive(frame.command_buffer, scene->getSkyboxMesh()->getPrimitive());

    driver->setShader(ShaderType::VERTEX, pbr_vert_shader->getShader());
    driver->setShader(ShaderType::FRAGMENT, pbr_frag_shader->getShader());
    driver->drawIndexedPrimitive(frame.command_buffer, scene->getMesh()->getPrimitive());
}


void Render::setEnvironment(const ApplicationResource *resource, uint8_t index)
{
    const VulkanTexture* environment_texture= resource->getHDRIEnvironmentubeMap(index);
    const VulkanTexture* irrandance_texture= resource->getIrridanceCubeMap(index);
    driver->bindTexture(scene_bind_set,5,environment_texture->getTexture());
    driver->bindTexture(scene_bind_set,6,irrandance_texture->getTexture());
}
