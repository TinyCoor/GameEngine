//
// Created by 12132 on 2021/11/27.
//
#include "../API.h"
#include "driver.h"
#include "VulkanUtils.h"
#include "VulkanContext.h"

#include <volk.h>
#include <cassert>
namespace render::backend::vulkan {

///todo array support
static VkFormat toVKFormat(render::backend::Format format) {
#define CASE_CONCAT(prefix, name) case name: return prefix##_##name;
  switch (format) {
  CASE_CONCAT(VK_FORMAT, UNDEFINED)
  CASE_CONCAT(VK_FORMAT, R8_UNORM)
  CASE_CONCAT(VK_FORMAT, R8_SNORM)
  CASE_CONCAT(VK_FORMAT, R8_UINT)
  CASE_CONCAT(VK_FORMAT, R8_SINT)
  CASE_CONCAT(VK_FORMAT, R8G8_UNORM)
  CASE_CONCAT(VK_FORMAT, R8G8_SNORM)
  CASE_CONCAT(VK_FORMAT, R8G8_UINT)
  CASE_CONCAT(VK_FORMAT, R8G8_SINT)
  CASE_CONCAT(VK_FORMAT, R8G8B8_UNORM)
  CASE_CONCAT(VK_FORMAT, R8G8B8_SNORM)
  CASE_CONCAT(VK_FORMAT, R8G8B8_UINT)
  CASE_CONCAT(VK_FORMAT, R8G8B8_SINT)
  CASE_CONCAT(VK_FORMAT, R8G8B8A8_UNORM)
  CASE_CONCAT(VK_FORMAT, R8G8B8A8_SNORM)
  CASE_CONCAT(VK_FORMAT, R8G8B8A8_UINT)
  CASE_CONCAT(VK_FORMAT, R8G8B8A8_SINT)

    // 16 bit
  CASE_CONCAT(VK_FORMAT, R16_UNORM)
  CASE_CONCAT(VK_FORMAT, R16_SNORM)
  CASE_CONCAT(VK_FORMAT, R16_UINT)
  CASE_CONCAT(VK_FORMAT, R16_SINT)
  CASE_CONCAT(VK_FORMAT, R16_SFLOAT)
  CASE_CONCAT(VK_FORMAT, R16G16_UNORM)
  CASE_CONCAT(VK_FORMAT, R16G16_SNORM)
  CASE_CONCAT(VK_FORMAT, R16G16_UINT)
  CASE_CONCAT(VK_FORMAT, R16G16_SINT)
  CASE_CONCAT(VK_FORMAT, R16G16_SFLOAT)
  CASE_CONCAT(VK_FORMAT, R16G16B16_UNORM)
  CASE_CONCAT(VK_FORMAT, R16G16B16_SNORM)
  CASE_CONCAT(VK_FORMAT, R16G16B16_UINT)
  CASE_CONCAT(VK_FORMAT, R16G16B16_SINT)
  CASE_CONCAT(VK_FORMAT, R16G16B16_SFLOAT)
  CASE_CONCAT(VK_FORMAT, R16G16B16A16_UNORM)
  CASE_CONCAT(VK_FORMAT, R16G16B16A16_SNORM)
  CASE_CONCAT(VK_FORMAT, R16G16B16A16_UINT)
  CASE_CONCAT(VK_FORMAT, R16G16B16A16_SINT)
  CASE_CONCAT(VK_FORMAT, R16G16B16A16_SFLOAT)

    //32 bits
  CASE_CONCAT(VK_FORMAT, R32_UINT)
  CASE_CONCAT(VK_FORMAT, R32_SINT)
  CASE_CONCAT(VK_FORMAT, R32_SFLOAT)
  CASE_CONCAT(VK_FORMAT, R32G32_UINT)
  CASE_CONCAT(VK_FORMAT, R32G32_SINT)
  CASE_CONCAT(VK_FORMAT, R32G32_SFLOAT)
  CASE_CONCAT(VK_FORMAT, R32G32B32_UINT)
  CASE_CONCAT(VK_FORMAT, R32G32B32_SINT)
  CASE_CONCAT(VK_FORMAT, R32G32B32_SFLOAT)
  CASE_CONCAT(VK_FORMAT, R32G32B32A32_UINT)
  CASE_CONCAT(VK_FORMAT, R32G32B32A32_SINT)
  CASE_CONCAT(VK_FORMAT, R32G32B32A32_SFLOAT)
    //depth
  CASE_CONCAT(VK_FORMAT, D16_UNORM)
  CASE_CONCAT(VK_FORMAT, D16_UNORM_S8_UINT)
  CASE_CONCAT(VK_FORMAT, D24_UNORM_S8_UINT)
  CASE_CONCAT(VK_FORMAT, D32_SFLOAT)
  CASE_CONCAT(VK_FORMAT, D32_SFLOAT_S8_UINT)

  }
#undef CASE_CONCAT

  return VK_FORMAT_UNDEFINED;
}

static VkIndexType ToVkIndexType(uint8_t index_size) {
  static VkIndexType supports[static_cast<unsigned int>(IndexFormat::MAX)] = {
      VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32
  };
  return supports[static_cast<uint32_t>(index_size)];
}

static VkPrimitiveTopology toPrimitiveTopology(RenderPrimitiveType type) {
  static VkPrimitiveTopology supports_type[static_cast<uint32_t>(RenderPrimitiveType::MAX)] = {
      // points
      VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
      // lines
      VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
      VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
      VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,

      //traiangles
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,

      // patches
      VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
  };
  return supports_type[static_cast<uint32_t>(type)];
}

static size_t toPixelSize(Format format) {
#define CASE_FORMAT_SIZE(format, count, type) case format : return sizeof(type) * count;
  switch (format) {
  CASE_FORMAT_SIZE(R8_UNORM, 1, uint8_t)
  CASE_FORMAT_SIZE(R8_SNORM, 1, uint8_t)
  CASE_FORMAT_SIZE(R8_UINT, 1, uint8_t)
  CASE_FORMAT_SIZE(R8_SINT, 1, uint8_t)

  CASE_FORMAT_SIZE(R8G8_UNORM, 2, uint8_t)
  CASE_FORMAT_SIZE(R8G8_SNORM, 2, uint8_t)
  CASE_FORMAT_SIZE(R8G8_UINT, 2, uint8_t)
  CASE_FORMAT_SIZE(R8G8_SINT, 2, uint8_t)

  CASE_FORMAT_SIZE(R8G8B8_UNORM, 3, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8_SNORM, 3, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8_UINT, 3, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8_SINT, 3, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8A8_UNORM, 4, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8A8_SNORM, 4, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8A8_UINT, 4, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8A8_SINT, 4, uint8_t)

  CASE_FORMAT_SIZE(R16_UNORM, 1, uint16_t)
  CASE_FORMAT_SIZE(R16_SNORM, 1, uint16_t)
  CASE_FORMAT_SIZE(R16_UINT, 1, uint16_t)
  CASE_FORMAT_SIZE(R16_SINT, 1, uint16_t)
  CASE_FORMAT_SIZE(R16_SFLOAT, 1, uint16_t)

  CASE_FORMAT_SIZE(R16G16_UNORM, 2, uint16_t)
  CASE_FORMAT_SIZE(R16G16_SNORM, 2, uint16_t)
  CASE_FORMAT_SIZE(R16G16_UINT, 2, uint16_t)
  CASE_FORMAT_SIZE(R16G16_SINT, 2, uint16_t)
  CASE_FORMAT_SIZE(R16G16_SFLOAT, 2, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16_UNORM, 3, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16_SNORM, 3, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16_UINT, 3, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16_SINT, 3, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16_SFLOAT, 3, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16A16_UNORM, 4, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16A16_SNORM, 4, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16A16_UINT, 4, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16A16_SINT, 4, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16A16_SFLOAT, 4, uint16_t)

  CASE_FORMAT_SIZE(R32_UINT, 1, uint32_t)
  CASE_FORMAT_SIZE(R32_SINT, 1, uint32_t)
  CASE_FORMAT_SIZE(R32_SFLOAT, 1, uint32_t)
  CASE_FORMAT_SIZE(R32G32_UINT, 2, uint32_t)
  CASE_FORMAT_SIZE(R32G32_SINT, 2, uint32_t)
  CASE_FORMAT_SIZE(R32G32_SFLOAT, 2, uint32_t)
  CASE_FORMAT_SIZE(R32G32B32_UINT, 3, uint32_t)
  CASE_FORMAT_SIZE(R32G32B32_SINT, 3, uint32_t)
  CASE_FORMAT_SIZE(R32G32B32_SFLOAT, 3, uint32_t)
  CASE_FORMAT_SIZE(R32G32B32A32_UINT, 4, uint32_t)
  CASE_FORMAT_SIZE(R32G32B32A32_SINT, 4, uint32_t)
  CASE_FORMAT_SIZE(R32G32B32A32_SFLOAT, 4, uint32_t)

    // depth formats
  CASE_FORMAT_SIZE(D16_UNORM, 2, uint8_t)
  CASE_FORMAT_SIZE(D16_UNORM_S8_UINT, 2, uint8_t)
  CASE_FORMAT_SIZE(D24_UNORM_S8_UINT, 3, uint8_t)
  CASE_FORMAT_SIZE(D32_SFLOAT, 4, uint8_t)
  CASE_FORMAT_SIZE(D32_SFLOAT_S8_UINT, 4, uint8_t)
  }
#undef CASE_FORMAT_SIZE
  return 0;
}

static void fillTexture(const VulkanContext *context, Texture *texture,
                        Format format, const void *data,
                        int num_data_mipmaps, int num_data_layer) {
  VulkanUtils::createImage(context, texture->type,
                           texture->width, texture->height, texture->depth,
                           texture->num_mipmaps, texture->num_layers, texture->samples,
                           texture->format, texture->tiling,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                               | VK_IMAGE_USAGE_SAMPLED_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture->flags,
                           texture->image, texture->imageMemory);
  if (data == nullptr) {
    VulkanUtils::fillImage(
        context, texture->image,
        texture->width, texture->height, texture->depth,
        texture->num_mipmaps, texture->num_layers, toPixelSize(format), data,
        texture->format, num_data_mipmaps, num_data_layer);
  } else {
    VulkanUtils::transitionImageLayout(context, texture->image, texture->format,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                       0, texture->num_mipmaps,
                                       0, texture->num_layers);
  }
}

VertexBuffer *Driver::createVertexBuffer(BufferType type,
                                         uint16_t vertex_size,
                                         uint32_t num_vertices,
                                         uint8_t num_attributes,
                                         const VertexAttribute *attributes,
                                         const void *data) {
  assert(type == BufferType::STATIC && "Dynamic are not impl");
  assert(num_vertices != 0 && data != nullptr && "Invalid data");
  assert(vertex_size != 0 && data != nullptr && "Invalid VertexSize");
  assert(num_attributes <= vulkan::VertexBuffer::MAX_ATTRIBUTES && "Vetex Attribute limit to 16");

  VkDeviceSize buffer_size = vertex_size * num_vertices;
  vulkan::VertexBuffer *result = new vulkan::VertexBuffer;
  result->vertex_size = vertex_size;
  result->num_vertices = num_vertices;
  result->num_attributes = num_attributes;
  memcpy(result->attributes, attributes, num_vertices * sizeof(VertexBuffer));

  VulkanUtils::createDeviceLocalBuffer(context, buffer_size,
                                       data,
                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                       result->buffer,
                                       result->memory);

  return result;
}

IndexBuffer *Driver::createIndexBuffer(BufferType type,
                                       uint8_t index_size,
                                       uint32_t num_indices,
                                       const void *data) {
  assert(type == BufferType::STATIC && "Dynamic are not impl");
  assert(num_indices != 0 && data != nullptr && "Invalid data");
  assert(static_cast<uint32_t>(index_size) != 0 && data != nullptr && "Invalid VertexSize");
  IndexBuffer *indexBuffer = new IndexBuffer();
  indexBuffer->num_indices = num_indices;
  indexBuffer->type = ToVkIndexType(index_size);
  VulkanUtils::createDeviceLocalBuffer(context,
                                       static_cast<uint32_t>(index_size),
                                       data,
                                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                       indexBuffer->buffer,
                                       indexBuffer->memory);

  return nullptr;
}

RenderPrimitive *Driver::createRenderPrimitive(RenderPrimitiveType type,
                                               const render::backend::VertexBuffer *vertex_buffer,
                                               const render::backend::IndexBuffer *index_buffer) {
  const vulkan::VertexBuffer *v_buffer = static_cast<const vulkan::VertexBuffer *>(vertex_buffer);
  const vulkan::IndexBuffer *i_buffer = static_cast<const vulkan::IndexBuffer *>(index_buffer);

  vulkan::RenderPrimitive *primitive = new vulkan::RenderPrimitive;
  primitive->vertexBuffer = v_buffer;
  primitive->indexBuffer = i_buffer;
  primitive->type = toPrimitiveTopology(type);
  return primitive;
}

Texture *Driver::createTexture2D(uint32_t width,
                                 uint32_t height,
                                 uint32_t num_mipmaps,
                                 Format format,
                                 const void *data,
                                 uint32_t num_data_mipmaps) {
  vulkan::Texture *texture = new vulkan::Texture();
  texture->width = width;
  texture->height = height;
  texture->format = toVKFormat(format);
  texture->num_mipmaps = num_mipmaps;
  texture->num_layers = 1;
  texture->type = VK_IMAGE_TYPE_2D;
  texture->samples = VK_SAMPLE_COUNT_1_BIT;
  texture->tiling = VK_IMAGE_TILING_OPTIMAL;
  texture->flags = 0;

  fillTexture(context, texture, format, data, num_data_mipmaps, 1);
  return texture;
}

Texture *Driver::createTexture2DArray(uint32_t width,
                                      uint32_t height,
                                      uint32_t num_mipmaps,
                                      uint32_t num_layers,
                                      Format format,
                                      const void *data,
                                      uint32_t num_data_mipmaps,
                                      uint32_t num_data_layers) {

  vulkan::Texture *texture = new vulkan::Texture();
  texture->type = VK_IMAGE_TYPE_2D;
  texture->width = width;
  texture->height = height;
  texture->depth = 1;
  texture->format = toVKFormat(format);
  texture->num_mipmaps = num_mipmaps;
  texture->num_layers = num_layers;
  texture->samples = VK_SAMPLE_COUNT_1_BIT;
  texture->tiling = VK_IMAGE_TILING_OPTIMAL;
  texture->flags = 0;

  fillTexture(context, texture, format, data, num_data_mipmaps, num_data_layers);

  return texture;
}

Texture *Driver::createTexture3D(uint32_t width,
                                 uint32_t height,
                                 uint32_t depth,
                                 uint32_t num_mipmaps,
                                 Format format,
                                 const void *data,
                                 uint32_t num_data_mipmaps) {
  vulkan::Texture *texture = new vulkan::Texture();
  texture->type = VK_IMAGE_TYPE_3D;
  texture->width = width;
  texture->height = height;
  texture->depth = depth;
  texture->format = toVKFormat(format);
  texture->num_mipmaps = num_mipmaps;
  texture->num_layers = 1;
  texture->samples = VK_SAMPLE_COUNT_1_BIT;
  texture->tiling = VK_IMAGE_TILING_OPTIMAL;
  texture->flags = 0;

  fillTexture(context, texture, format, data, num_data_mipmaps, 1);
  return texture;
}

Texture *Driver::createTextureCube(uint32_t width,
                                   uint32_t height,
                                   uint32_t num_mipmaps,
                                   Format format,
                                   const void *data,
                                   uint32_t num_data_mipmaps) {
  vulkan::Texture *texture = new vulkan::Texture();
  texture->type = VK_IMAGE_TYPE_2D;
  texture->width = width;
  texture->height = height;
  texture->depth = 1;
  texture->format = toVKFormat(format);
  texture->num_mipmaps = num_mipmaps;
  texture->num_layers = 6;
  texture->samples = VK_SAMPLE_COUNT_1_BIT;
  texture->tiling = VK_IMAGE_TILING_OPTIMAL;
  texture->flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

  fillTexture(context, texture, format, data, num_data_mipmaps, 1);
  return texture;
}

FrameBuffer *Driver::createFrameBuffer(uint8_t num_color_attachments,
                                       const FrameBufferColorAttachment *color_attachments,
                                       const FrameBufferDepthStencilAttachment *depthstencil_attachment) {
  return nullptr;
}
UniformBuffer *Driver::createUniformBuffer(BufferType type, uint32_t size, const void *data) {
  return nullptr;
}
Shader *Driver::createShaderFromSource(ShaderType type, uint32_t length, const char *source) {
  return nullptr;
}
Shader *Driver::createShaderFromBytecode(ShaderType type, uint32_t size, const void *data) {
  return nullptr;
}
void Driver::destroyVertexBuffer(render::backend::VertexBuffer *vertex_buffer) {

}
void Driver::destroyIndexBuffer(render::backend::IndexBuffer *index_buffer) {

}
void Driver::destroyRenderPrimitive(render::backend::RenderPrimitive *render_primitive) {

}
void Driver::destroyTexture(render::backend::Texture *texture) {

}
void Driver::destroyFrameBuffer(render::backend::FrameBuffer *frame_buffer) {

}
void Driver::destroyUniformBuffer(render::backend::UniformBuffer *uniform_buffer) {

}
void Driver::destroyShader(render::backend::Shader *shader) {

}
void Driver::beginRenderPass(const render::backend::FrameBuffer *frame_buffer) {

}
void Driver::endRenderPass() {

}
void Driver::bindUniformBuffer(uint32_t unit, const render::backend::UniformBuffer *uniform_buffer) {

}
void Driver::bindTexture(uint32_t unit, const render::backend::Texture *texture) {

}
void Driver::bindShader(const render::backend::Shader *shader) {

}
void Driver::drawIndexedPrimitive(const render::backend::RenderPrimitive *render_primitive) {

}
void Driver::drawIndexedPrimitiveInstanced(const render::backend::RenderPrimitive *primitive,
                                           const render::backend::VertexBuffer *instance_buffer,
                                           uint32_t num_instances,
                                           uint32_t offset) {

}
}
