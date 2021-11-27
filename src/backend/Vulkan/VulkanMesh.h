//
// Created by y123456 on 2021/10/16.
//

#ifndef GAMEENGINE_VULKANRENDERMODEL_H
#define GAMEENGINE_VULKANRENDERMODEL_H
#include <vector>
#include "VulkanContext.h"
#include "../core/Vertex.h"

///GPU顶点数据
namespace render::backend::vulkan {
class VulkanMesh {
private:

  const VulkanContext *context;
  std::vector<core::Vertex> vertices;
  std::vector<uint32_t> indices;

  VkBuffer vertexBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

  VkBuffer indexBuffer = VK_NULL_HANDLE;
  VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

public:
  VulkanMesh(const VulkanContext *ctx) : context(ctx) {}
  ~VulkanMesh();

  inline uint32_t getNumIndices() const { return indices.size(); }
  inline VkBuffer getVertexBuffer() const { return vertexBuffer; }
  inline VkBuffer getIndexBuffer() const { return indexBuffer; }

  static VkVertexInputBindingDescription getVertexInputBindingDescription();
  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

  bool loadFromFile(const char *file);

  void createSkybox(float size);
  void createQuad(float size);

  void uploadToGPU();
  void clearGPUData();
  void clearCPUData();
private:
  void createIndexBuffer();
  void createVertexBuffer();
};
}
#endif //GAMEENGINE_VULKANRENDERMODEL_H
