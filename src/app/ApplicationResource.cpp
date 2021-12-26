//
// Created by y123456 on 2021/10/12.
//

#include "ApplicationResource.h"
#include "../backend/Vulkan/RenderUtils.h"

namespace render::backend::vulkan {

ApplicationResource::~ApplicationResource()
{
    shutdown();
}

void ApplicationResource::init()
{
    for (int i = 0; i < config::shaders.size(); ++i) {
        resources.loadShader(i, config::shaderTypes[i], config::shaders[i]);
    }
    for (int i = 0; i < config::textures.size(); ++i) {
        resources.loadTexture(i, config::textures[i]);
    }

    for (int i = 0; i < config::meshes.size(); ++i) {
        resources.loadMesh(i, config::meshes[i]);
    }

    for (int i = 0; i < config::hdrTextures.size(); ++i) {
        resources.loadTexture(config::Textures::EnvironmentBase + i, config::hdrTextures[i]);
    }

    resources.createCubeMesh(config::Meshes::Skybox, 1000.0);
    baked_brdf = RenderUtils::createTexture2D(driver, Format::R16G16_SFLOAT,
                                              256, 256, 1,
                                              resources.getShader(config::Shaders::BakedBRDFVertex),
                                              resources.getShader(config::Shaders::BakedBRDFFrag));

    environment_cubemaps.resize(config::hdrTextures.size());
    irrandance_cubemaps.resize(config::hdrTextures.size());
    for (int i = 0; i < config::hdrTextures.size(); ++i) {
        environment_cubemaps[i] = RenderUtils::hdriToCube(driver,
                                                          Format::R32G32B32A32_SFLOAT,
                                                          256,
                                                          resources.getTexture(config::Textures::EnvironmentBase + i),
                                                          resources.getShader(config::Shaders::CubeVertex),
                                                          resources.getShader(config::Shaders::hdriToCubeFrag),
                                                          resources.getShader(config::Shaders::CubeToPrefilteredSpecular));

        irrandance_cubemaps[i] = RenderUtils::createTextureCube(driver,
                                                                Format::R32G32B32A32_SFLOAT,
                                                                256, 1,
                                                                resources.getShader(config::Shaders::CubeVertex),
                                                                resources.getShader(config::Shaders::diffuseIrrandianceFrag),
                                                                environment_cubemaps[i]);

    }

}

void ApplicationResource::shutdown()
{
    resources.shutdown();
    //driver->destroyTexture(const_cast<backend::Texture *>(baked_brdf->getTexture()));
    delete baked_brdf;
    for (int i = 0; i <environment_cubemaps.size() ; ++i) {
//        driver->destroyTexture(environment_cubemaps[i]);
        delete environment_cubemaps[i];
    }

    for (int i = 0; i <irrandance_cubemaps.size() ; ++i) {
//        driver->destroyTexture(environment_cubemaps[i]);
        delete irrandance_cubemaps[i];
    }
    environment_cubemaps.clear();
    irrandance_cubemaps.clear();
}

bool ApplicationResource::reloadShader()
{
    for (int i = 0; i < config::shaders.size(); ++i) {
        if (resources.reloadShader(i)) {
            return true;
        }
    }
    return false;
}

}
