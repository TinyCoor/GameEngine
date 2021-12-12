//
// Created by y123456 on 2021/10/12.
//

#ifndef GAMEENGINE_RENDER_DATA_H
#define GAMEENGINE_RENDER_DATA_H
#include <volk.h>
#include "../backend/Vulkan/VulkanResourceManager.h"
#include "config.h"
namespace render::backend::vulkan {
class VulkanRenderScene {
private:
  VulkanResourceManager resources;
public:
  VulkanRenderScene(render::backend::Driver *driver)
  : resources(driver) { }
  ~VulkanRenderScene();

  void init();
  void shutdown();

  inline const VulkanResourceManager &getResource() const { return resources; }

  inline VulkanShader* getPBRVertexShader() { return resources.getShader(config::Shaders::PBRVertex); }
  inline VulkanShader* getPBRFragmentShader() { return resources.getShader(config::Shaders::PBRFrag); }

  inline VulkanShader* getSkyboxVertexShader() { return resources.getShader(config::Shaders::SkyboxVertex); }
  inline VulkanShader* getSkyboxFragmentShader() { return resources.getShader(config::Shaders::SkyboxFrag); }
  inline VulkanShader* getCubeToPrefilteredSpecularShader() const { return resources.getShader(config::Shaders::CubeToPrefilteredSpecular); }

  inline VulkanShader* getCubeVertexShader() { return resources.getShader(config::Shaders::CubeVertex); }
  inline VulkanShader* getHDRToCubeFragmentShader() { return resources.getShader(config::Shaders::hdriToCubeFrag); }
  inline VulkanShader* getDiffuseToIrridanceShader() { return resources.getShader(config::Shaders::diffuseIrrandianceFrag); }

  inline VulkanShader* getBakedVertexShader() { return resources.getShader(config::Shaders::BakedBRDFVertex); }
  inline VulkanShader* getBakedFragmentShader() { return resources.getShader(config::Shaders::BakedBRDFFrag); }

  inline VulkanTexture* getEmissionTexture() { return resources.getTexture(config::Textures::emissionTexture); }
  inline VulkanTexture* getAlbedoTexture() { return resources.getTexture(config::Textures::albedoTexture); }
  inline VulkanTexture* getHDRTexture(int index) const {
    return resources.getHDRTexture(config::Textures::EnvironmentBase + index);
  }
  inline VulkanTexture* getNormalTexture() { return resources.getTexture(config::Textures::normalTexture); }
  inline VulkanTexture* getAOTexture() { return resources.getTexture(config::Textures::aoTexture); }
  inline VulkanTexture* getShadingTexture() { return resources.getTexture(config::Textures::shadingTexture); }
  inline VulkanMesh* getMesh() { return resources.getMesh(config::Meshes::SciFiHelmet); }
  inline VulkanMesh* getSkyboxMesh() { return resources.getMesh(config::Meshes::Skybox); }

  inline size_t getNumHDRTextures() const { return config::hdrTextures.size(); }
  inline const char *getHDRTexturePath(int index) const { return config::hdrTextures[index]; }

  bool reloadShader();

};
}
#endif //GAMEENGINE_RENDER_DATA_H
