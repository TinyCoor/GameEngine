//
// Created by y123456 on 2021/10/16.
//

#ifndef GAMEENGINE_VULKANRENDERMODEL_H
#define GAMEENGINE_VULKANRENDERMODEL_H
#include <vector>
#include "Device.h"
#include "../core/Mesh.h"
#include "driver.h"

class aiMesh;

///GPU顶点数据
namespace render::backend::vulkan {
class VulkanMesh {
private:
  render::backend::Driver *driver {nullptr};
  std::vector<core::Vertex> vertices;
  std::vector<uint32_t> indices;

  render::backend::VertexBuffer *vertex_buffer {nullptr};
  render::backend::IndexBuffer *index_buffer {nullptr};

  render::backend::RenderPrimitive* primitive{nullptr};

public:
  VulkanMesh(render::backend::Driver *driver)
      : driver(driver) { }
  ~VulkanMesh();

  inline uint32_t getNumIndices() const { return indices.size(); }
  VkBuffer getVertexBuffer() const ;
  VkBuffer getIndexBuffer() const ;

  static VkVertexInputBindingDescription getVertexInputBindingDescription();
  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
  inline render::backend::RenderPrimitive* getPrimitive() const {return primitive;}

  bool import(const char *file);
  bool import(const aiMesh* mesh);

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
