//
// Created by y123456 on 2021/10/12.
//

#include "VulkanRenderScene.h"
namespace render::backend::vulkan {

VulkanRenderScene::~VulkanRenderScene() {
  shutdown();
}

void VulkanRenderScene::init() {
  for (int i = 0; i < config::shaders.size(); ++i) {
    resources.loadShader(i, config::shaderTypes[i],config::shaders[i]);
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

}

void VulkanRenderScene::shutdown() {
  resources.shutdown();
}

bool VulkanRenderScene::reloadShader() {
  for (int i = 0; i < config::shaders.size(); ++i) {
    if (resources.reloadShader(i)) {
      return true;
    }
  }
  return false;
}

}
