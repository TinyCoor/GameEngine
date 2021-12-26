//
// Created by y123456 on 2021/10/12.
//

#ifndef GAMEENGINE_RENDER_DATA_H
#define GAMEENGINE_RENDER_DATA_H
#include <volk.h>
#include "../backend/Vulkan/VulkanResourceManager.h"
#include "config.h"
namespace render::backend::vulkan {
class ApplicationResource {
private:
    render::backend::Driver* driver{nullptr};
    VulkanResourceManager resources;
    VulkanTexture* baked_brdf{nullptr};
    std::vector<VulkanTexture*> environment_cubemaps;
    std::vector<VulkanTexture*> irrandance_cubemaps;
public:
    ApplicationResource(render::backend::Driver *driver) :driver(driver),resources(driver)
    {}
    ~ApplicationResource();

    void init();
    void shutdown();

    ///todo backend type
    inline const VulkanResourceManager &getResource() const { return resources; }

    inline const VulkanShader *getPBRVertexShader() const { return resources.getShader(config::Shaders::PBRVertex); }
    inline const VulkanShader *getPBRFragmentShader()const { return resources.getShader(config::Shaders::PBRFrag); }

    inline const VulkanShader *getGbufferVertexShader() const{ return resources.getShader(config::Shaders::GBufferVert); }
    inline const VulkanShader *getGbufferFragmentShader() const{ return resources.getShader(config::Shaders::GBufferFrag); }

    inline const VulkanShader *getSkyboxVertexShader()const { return resources.getShader(config::Shaders::SkyboxVertex); }
    inline const VulkanShader *getSkyboxFragmentShader()const { return resources.getShader(config::Shaders::SkyboxFrag); }
    inline const VulkanShader *getCubeToPrefilteredSpecularShader() const { return resources.getShader(config::Shaders::CubeToPrefilteredSpecular); }

    inline const VulkanShader *getCubeVertexShader()const { return resources.getShader(config::Shaders::CubeVertex); }
    inline const VulkanShader *getHDRToCubeFragmentShader()const { return resources.getShader(config::Shaders::hdriToCubeFrag); }
    inline const VulkanShader *getDiffuseToIrridanceShader()const { return resources.getShader(config::Shaders::diffuseIrrandianceFrag); }

    inline const VulkanShader *getBakedVertexShader() const{ return resources.getShader(config::Shaders::BakedBRDFVertex); }
    inline const VulkanShader *getBakedFragmentShader()const { return resources.getShader(config::Shaders::BakedBRDFFrag); }

    inline const VulkanTexture *getEmissionTexture()const { return resources.getTexture(config::Textures::emissionTexture); }
    inline const VulkanTexture *getAlbedoTexture()const { return resources.getTexture(config::Textures::albedoTexture); }
    inline const VulkanTexture *getHDRTexture(int index) const{ return resources.getHDRTexture(config::Textures::EnvironmentBase + index);}
    inline const VulkanTexture *getNormalTexture()const { return resources.getTexture(config::Textures::normalTexture); }
    inline const VulkanTexture *getAOTexture()const { return resources.getTexture(config::Textures::aoTexture); }
    inline const VulkanTexture *getShadingTexture() const{ return resources.getTexture(config::Textures::shadingTexture); }
    inline const VulkanTexture *getHDRIEnvironmentubeMap(int index) const{ return environment_cubemaps[index];}
    inline const VulkanTexture *getIrridanceCubeMap(int index) const{ return irrandance_cubemaps[index];}
    inline const VulkanTexture *getBakedBRDF() const{ return baked_brdf;}


    inline const VulkanMesh *getMesh()const { return resources.getMesh(config::Meshes::SciFiHelmet); }
    inline const VulkanMesh *getSkyboxMesh()const { return resources.getMesh(config::Meshes::Skybox); }


    inline size_t getNumHDRTextures() const { return config::hdrTextures.size(); }
    inline const char *getHDRTexturePath(int index) const { return config::hdrTextures[index]; }
    bool reloadShader();

};
}
#endif //GAMEENGINE_RENDER_DATA_H
