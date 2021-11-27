//
// Created by y123456 on 2021/10/16.
//

#include "VulkanMesh.h"
#include "VulkanUtils.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <assimp/cimport.h>
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
      {5, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(core::Vertex, uv)},
  };

  return attributes;
}

void VulkanMesh::createVertexBuffer() {
  VkDeviceSize bufferSize = sizeof(core::Vertex) * vertices.size();
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  VulkanUtils::createBuffer(context, bufferSize,
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            vertexBuffer,
                            vertexBufferMemory);

  VulkanUtils::createBuffer(context, bufferSize,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            stagingBuffer,
                            stagingBufferMemory);

  void *data = nullptr;
  vkMapMemory(context->Device(), stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), bufferSize);
  vkUnmapMemory(context->Device(), stagingBufferMemory);

  VulkanUtils::copyBuffer(context, stagingBuffer, vertexBuffer, bufferSize);

  vkDestroyBuffer(context->Device(), stagingBuffer, nullptr);
  vkFreeMemory(context->Device(), stagingBufferMemory, nullptr);
}

void VulkanMesh::createIndexBuffer() {
  //Create Index Buffer
  VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  VulkanUtils::createBuffer(context, bufferSize,
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            indexBuffer,
                            indexBufferMemory);

  VulkanUtils::createBuffer(context, bufferSize,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            stagingBuffer,
                            stagingBufferMemory);

  void *data = nullptr;
  vkMapMemory(context->Device(), stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), bufferSize);
  vkUnmapMemory(context->Device(), stagingBufferMemory);

  VulkanUtils::copyBuffer(context, stagingBuffer, indexBuffer, bufferSize);

  vkDestroyBuffer(context->Device(), stagingBuffer, nullptr);
  vkFreeMemory(context->Device(), stagingBufferMemory, nullptr);
}

//This is a bug in load form File
bool VulkanMesh::loadFromFile(const char *file) {
  clearCPUData();
  clearGPUData();
  core::loadMesh(file, vertices, indices);
  uploadToGPU();
  return true;
}

void VulkanMesh::clearGPUData() {
  vkDestroyBuffer(context->Device(), vertexBuffer, nullptr);
  vkFreeMemory(context->Device(), vertexBufferMemory, nullptr);
  vkDestroyBuffer(context->Device(), indexBuffer, nullptr);
  vkFreeMemory(context->Device(), indexBufferMemory, nullptr);
  vertexBuffer = VK_NULL_HANDLE;
  vertexBufferMemory = VK_NULL_HANDLE;
  indexBuffer = VK_NULL_HANDLE;
  indexBufferMemory = VK_NULL_HANDLE;
}

void VulkanMesh::uploadToGPU() {
  if (!vertices.empty()) {
    createVertexBuffer();
  }
  if (!indices.empty()) {
    createIndexBuffer();
  }
}

void VulkanMesh::clearCPUData() {
  vertices.clear();
  indices.clear();
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