//
// Created by 12132 on 2021/10/30.
//

#ifndef GAMEENGINE_VULKANRESOURCEMANAGER_H
#define GAMEENGINE_VULKANRESOURCEMANAGER_H
#include "Device.h"
#include "VulkanShader.h"
#include <unordered_map>
#include <memory>
namespace render::backend::vulkan {
class VulkanShader;
class VulkanTexture;
class VulkanMesh;
class Device;

class VulkanResourceManager {
public:
  VulkanResourceManager(render::backend::Driver *driver)
      : driver(driver) {}
  ~VulkanResourceManager();

  VulkanShader *loadShader(int id, render::backend::ShaderType type, const char *path);
  bool reloadShader(int id);
  VulkanMesh* loadMesh(int id, const char *path);
  VulkanMesh* createCubeMesh(int id, float size);
  VulkanTexture* loadTexture(int id, const char *path);

  VulkanShader* getShader(int id) const;
  VulkanTexture* getTexture(int id) const;
  VulkanMesh* getMesh(int id) const;
  VulkanTexture* getHDRTexture(int id) const;

  void shutdown();

private:

  render::backend::Driver *driver {nullptr};
  std::unordered_map<int, VulkanShader*> shaders;
  std::unordered_map<int, VulkanMesh*> meshes;
  std::unordered_map<int, VulkanTexture*> textures;
};
}

#endif //GAMEENGINE_VULKANRESOURCEMANAGER_H
