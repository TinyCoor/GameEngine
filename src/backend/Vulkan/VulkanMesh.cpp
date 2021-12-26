//
// Created by y123456 on 2021/10/16.
//

#include "VulkanMesh.h"
#include "Utils.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

namespace render::backend::vulkan {
VulkanMesh::~VulkanMesh() {
  clearGPUData();
  clearCPUData();
}

VkVertexInputBindingDescription VulkanMesh::getVertexInputBindingDescription() {
  static VkVertexInputBindingDescription bindingDescription = {
      .binding =0,
      .stride = sizeof(core::Vertex),
      .inputRate =VK_VERTEX_INPUT_RATE_VERTEX
  };
  return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> VulkanMesh::getAttributeDescriptions() {
  static std::vector<VkVertexInputAttributeDescription> attributes{
      {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(core::Vertex, position)},
      {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(core::Vertex, tangent)},
      {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(core::Vertex, binormal)},
      {3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(core::Vertex, normal)},
      {4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(core::Vertex, color)},
      {5, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(core::Vertex, uv)},
  };

  return attributes;
}

void VulkanMesh::createVertexBuffer() {

  static render::backend::VertexAttribute attributes[] =
      {
          { render::backend::Format::R32G32B32_SFLOAT, offsetof(core::Vertex, position) },
          { render::backend::Format::R32G32B32_SFLOAT, offsetof(core::Vertex, tangent) },
          { render::backend::Format::R32G32B32_SFLOAT, offsetof(core::Vertex, binormal) },
          { render::backend::Format::R32G32B32_SFLOAT, offsetof(core::Vertex, normal) },
          { render::backend::Format::R32G32B32_SFLOAT, offsetof(core::Vertex, color) },
          { render::backend::Format::R32G32_SFLOAT,    offsetof(core::Vertex, uv) },
      };

  vertex_buffer = driver->createVertexBuffer(
      render::backend::BufferType::STATIC,
      sizeof(core::Vertex), static_cast<uint32_t>(vertices.size()),
      6, attributes,
      vertices.data()
  );
}

VkBuffer VulkanMesh::getVertexBuffer() const {
  if (vertex_buffer == nullptr)
    return VK_NULL_HANDLE;

  return static_cast<const render::backend::vulkan::VertexBuffer *>(vertex_buffer)->buffer;

}
VkBuffer VulkanMesh::getIndexBuffer() const {
  if (index_buffer == nullptr)
    return VK_NULL_HANDLE;

  return static_cast<const render::backend::vulkan::IndexBuffer *>(index_buffer)->buffer;

}

void VulkanMesh::createIndexBuffer() {
  index_buffer = driver->createIndexBuffer(
      render::backend::BufferType::STATIC,
      render::backend::IndexSize::UINT32,
      static_cast<uint32_t>(indices.size()),
      indices.data()
  );
}

bool VulkanMesh::import(const char *file) {
  clearGPUData();
  core::import(file, vertices, indices);
  uploadToGPU();
  return true;
}

bool VulkanMesh::import(const aiMesh *mesh)
{
    clearGPUData();
    core::import(mesh, vertices, indices);
    uploadToGPU();
    return true;
}

void VulkanMesh::clearGPUData() {
  driver->destroyVertexBuffer(vertex_buffer);
  vertex_buffer = nullptr;
  driver->destroyIndexBuffer(index_buffer);
  index_buffer = nullptr;
}

void VulkanMesh::uploadToGPU() {
    createVertexBuffer();
    createIndexBuffer();
    primitive = driver->createRenderPrimitive(render::backend::RenderPrimitiveType::TRIANGLE_LIST,
                                              vertex_buffer,index_buffer);

}

void VulkanMesh::clearCPUData() {
  vertices.clear();
  indices.clear();
  driver->destroyRenderPrimitive(primitive);
  primitive = nullptr;
}

void VulkanMesh::createSkybox(float size) {
  size *= 0.5;
  vertices.resize(8);
  indices.resize(36);
  vertices[0].position = glm::vec3(-size, -size, -size);
  vertices[1].position = glm::vec3(size, -size, -size);
  vertices[2].position = glm::vec3(size, size, -size);
  vertices[3].position = glm::vec3(-size, size, -size);

  vertices[4].position = glm::vec3(-size, -size, size);
  vertices[5].position = glm::vec3(size, -size, size);
  vertices[6].position = glm::vec3(size, size, size);
  vertices[7].position = glm::vec3(-size, size, size);

  indices = {
      0, 1, 2, 2, 3, 0,
      1, 5, 6, 6, 2, 1,
      3, 2, 6, 6, 7, 3,
      5, 4, 6, 4, 7, 6,
      1, 0, 4, 4, 5, 1,
      4, 0, 3, 3, 7, 4,
  };

  uploadToGPU();
}

void VulkanMesh::createQuad(float size) {

  float halfSize = size * 0.5f;
  vertices.resize(4);
  indices.resize(6);

  vertices[0].position = glm::vec3(-halfSize, -halfSize, 0.0f);
  vertices[1].position = glm::vec3(halfSize, -halfSize, 0.0f);
  vertices[2].position = glm::vec3(halfSize, halfSize, 0.0f);
  vertices[3].position = glm::vec3(-halfSize, halfSize, 0.0f);

  vertices[0].uv = glm::vec2(0.0f, 0.0f);
  vertices[1].uv = glm::vec2(1.0f, 0.0f);
  vertices[2].uv = glm::vec2(1.0f, 1.0f);
  vertices[3].uv = glm::vec2(0.0f, 1.0f);

  indices = {
      1, 0, 2, 3, 2, 0,
  };

  uploadToGPU();
}

}