//
// Created by 12132 on 2021/10/30.
//

#ifndef GAMEENGINE_VULKANRESOURCEMANAGER_H
#define GAMEENGINE_VULKANRESOURCEMANAGER_H
#include "VulkanContext.h"
#include "VulkanShader.h"
#include <unordered_map>
#include <memory>
namespace render::backend::vulkan {
class VulkanShader;
class VulkanTexture;
class VulkanMesh;
class VulkanContext;

//TODO Make User defined Deconstructor
class VulkanResourceManager {
public:
  VulkanResourceManager(const VulkanContext *ctx);
  ~VulkanResourceManager();

  std::shared_ptr<VulkanShader> loadShader(int id, ShaderKind kind, const char *path);
  std::shared_ptr<VulkanShader> loadShader(int id, const char *path);
  bool reloadShader(int id);
  std::shared_ptr<VulkanMesh> loadMesh(int id, const char *path);
  std::shared_ptr<VulkanMesh> createCubeMesh(int id, float size);
  std::shared_ptr<VulkanTexture> loadTexture(int id, const char *path);

  std::shared_ptr<VulkanShader> getShader(int id) const;
  std::shared_ptr<VulkanTexture> getTexture(int id) const;
  std::shared_ptr<VulkanMesh> getMesh(int id) const;
  std::shared_ptr<VulkanTexture> getHDRTexture(int id) const;

  void shutdown();

private:
  //May be This is singleton
  const VulkanContext *context;
  std::unordered_map<int, std::shared_ptr<VulkanShader>> shaders;
  std::unordered_map<int, std::shared_ptr<VulkanMesh>> meshes;
  std::unordered_map<int, std::shared_ptr<VulkanTexture>> textures;
};
}

#endif //GAMEENGINE_VULKANRESOURCEMANAGER_H
