//
// Created by y123456 on 2021/10/12.
//

#ifndef GAMEENGINE_RENDER_DATA_H
#define GAMEENGINE_RENDER_DATA_H
#include "volk.h"
#include "ResourceManager.h"

#include "config.h"

class VulkanRenderScene {
private:
    VulkanRenderContext context;
    VulkanResourceManager resource;


public:
    explicit VulkanRenderScene(VulkanRenderContext& ctx);

    void init();

    inline std::shared_ptr<VulkanShader> getPBRVertexShader(){ return resource.getShader(config::Shaders::PBRVertex);}
    inline std::shared_ptr<VulkanShader> getPBRFragmentShader(){return resource.getShader(config::Shaders::PBRFrag);}
    inline std::shared_ptr<VulkanShader> getCubeVertexShader() {return resource.getShader(config::Shaders::CubeVertex);}
    inline std::shared_ptr<VulkanShader> getHDRToCubeFragmentShader(){return resource.getShader(config::Shaders::hdriToCubeFrag);}
    inline std::shared_ptr<VulkanShader> getDiffuseToIrridanceShader(){return resource.getShader(config::Shaders::diffuseIrrandianceFrag);}
    inline std::shared_ptr<VulkanShader> getSkyboxVertexShader(){ return resource.getShader(config::Shaders::SkyboxVertex);}
    inline std::shared_ptr<VulkanShader> getSkyboxFragmentShader() {return resource.getShader(config::Shaders::SkyboxFrag);}

    inline std::shared_ptr<VulkanTexture> getEmissionTexture()  { return resource.getTexture(config::Textures::emissionTexture); }
    inline std::shared_ptr<VulkanTexture> getAlbedoTexture() { return resource.getTexture(config::Textures::albedoTexture); }
    inline std::shared_ptr<VulkanTexture> getHDRTexture() { return  resource.getHDRTexture(config::Textures::Environment);}
    inline std::shared_ptr<VulkanTexture> getNormalTexture()  { return resource.getTexture(config::Textures::normalTexture); }
    inline std::shared_ptr<VulkanTexture> getAOTexture() { return resource.getTexture(config::Textures::aoTexture); }
    inline std::shared_ptr<VulkanTexture> getShadingTexture() { return resource.getTexture(config::Textures::shadingTexture); }
//
   inline std::shared_ptr<VulkanMesh> getMesh()  {return resource.getMesh(config::Meshes::SciFiHelmet);}
   inline std::shared_ptr<VulkanMesh> getSkyboxMesh()  {return resource.getMesh(config::Meshes::Skybox);}


    void shutdown() ;

};

#endif //GAMEENGINE_RENDER_DATA_H
