//
// Created by 12132 on 2021/11/27.
//

#ifndef GAMEENGINE_DRIVER_H
#define GAMEENGINE_DRIVER_H
#include "../API.h"
#include <volk.h>
namespace render::backend::vulkan {

struct VertexBuffer : public render::backend::VertexBuffer {
  static constexpr int MAX_ATTRIBUTES = 10;
  VkBuffer buffer{VK_NULL_HANDLE};
  VkDeviceMemory memory{VK_NULL_HANDLE};
  uint16_t vertex_size{0};
  uint32_t num_vertices{0};
  uint8_t num_attributes{0};
  VkVertexInputAttributeDescription attributes[MAX_ATTRIBUTES];

};

struct IndexBuffer : public render::backend::IndexBuffer {
  VkBuffer buffer{VK_NULL_HANDLE};
  VkDeviceMemory memory{VK_NULL_HANDLE};
  VkIndexType type{VK_INDEX_TYPE_UINT16};
  uint32_t num_indices;

};

struct RenderPrimitive : public render::backend::RenderPrimitive {
  VkPrimitiveTopology type{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
  const VertexBuffer *vertexBuffer{nullptr};
  const IndexBuffer *indexBuffer{nullptr};
};

struct Texture : public render::backend::Texture {
  VkImage image{VK_NULL_HANDLE};
  VkDeviceMemory imageMemory{VK_NULL_HANDLE};
  VkImageType type{VK_IMAGE_TYPE_2D};
  VkFormat format{VK_FORMAT_R8G8B8A8_UNORM};
  size_t width;
  size_t height;
  size_t depth;
  uint8_t num_layers;
  uint8_t num_mipmaps;
  VkImageTiling tiling{};
  VkSampleCountFlagBits samples;
};

struct RenderTarget : public render::backend::RenderTarget {
  static constexpr int MAX_COLOR_ATTATCHMENTS = 16;
  VkFramebuffer frameBuffer{VK_NULL_HANDLE};
  uint8_t num_color_attachment{0};
  VkImageView color_attachment[MAX_COLOR_ATTATCHMENTS];
  VkImageView depth_attachment{VK_NULL_HANDLE};
};

struct UniformBuffer : public render::backend::UniformBuffer {
  VkBuffer buffer{VK_NULL_HANDLE};
  VkDeviceMemory memory{VK_NULL_HANDLE};
  VkIndexType type{VK_INDEX_TYPE_UINT16};
  uint32_t num_indices{0};

};

struct Shader : public render::backend::Shader {
  ShaderType type{ShaderType::Fragment};
  VkShaderModule shaderModule{VK_NULL_HANDLE};

};

struct GraphicsProgram : public render::backend::GraphicsProgram {
  const Shader *vertex;
  const Shader *tessellation_control;
  const Shader *tessellation_evaulation;
  const Shader *geometry;
  const Shader *fragment;
};

class VulkanContext;
class Driver : public render::backend::Driver {
  const VulkanContext* context {nullptr};
public:

  VertexBuffer *createVertexBuffer(
      BufferType type,
      uint16_t vertex_size,
      uint32_t num_vertices,
      uint8_t num_attributes,
      const VertexAttribute *attributes,
      const void *data
  ) override;

  IndexBuffer *createIndexBuffer(
      BufferType type,
      uint8_t index_size,
      uint32_t num_indices,
      const void *data
  ) override;

  RenderPrimitive *createRenderPrimitive(
      RenderPrimitiveType type,
      const render::backend::VertexBuffer *vertex_buffer,
      const render::backend::IndexBuffer *index_buffer
  ) override;

  Texture *createTexture2D(
      uint32_t width,
      uint32_t height,
      uint32_t num_mipmaps,
      Format format,
      const void *data = nullptr,
      uint32_t num_data_mipmaps = 1
  ) override;

  Texture *createTexture2DArray(
      uint32_t width,
      uint32_t height,
      uint32_t num_mipmaps,
      uint32_t num_layers,
      Format format,
      const void *data = nullptr,
      uint32_t num_data_mipmaps = 1,
      uint32_t num_data_layers = 1
  ) override;

  Texture *createTexture3D(
      uint32_t width,
      uint32_t height,
      uint32_t depth,
      uint32_t num_mipmaps,
      Format format,
      const void *data = nullptr,
      uint32_t num_data_mipmaps = 1
  ) override;

  Texture *createTextureCube(
      uint32_t width,
      uint32_t height,
      uint32_t num_mipmaps,
      Format format,
      const void *data = nullptr,
      uint32_t num_data_mipmaps = 1
  ) override;

  FrameBuffer *createFrameBuffer(
      uint8_t num_color_attachments,
      const FrameBufferColorAttachment *color_attachments,
      const FrameBufferDepthStencilAttachment *depthstencil_attachment = nullptr
  ) override;

  UniformBuffer *createUniformBuffer(
      BufferType type,
      uint32_t size,
      const void *data
  ) override;

  Shader *createShaderFromSource(
      ShaderType type,
      uint32_t length,
      const char *source
  ) override;

  Shader *createShaderFromBytecode(
      ShaderType type,
      uint32_t size,
      const void *data
  ) override;

  void destroyVertexBuffer(render::backend::VertexBuffer *vertex_buffer) override;

  void destroyIndexBuffer(render::backend::IndexBuffer *index_buffer) override;

  void destroyRenderPrimitive(render::backend::RenderPrimitive *render_primitive) override;

  void destroyTexture(render::backend::Texture *texture) override;

  void destroyFrameBuffer(render::backend::FrameBuffer *frame_buffer) override;

  void destroyUniformBuffer(render::backend::UniformBuffer *uniform_buffer) override;

  void destroyShader(render::backend::Shader *shader) override;

public:

  // sequence
  virtual void beginRenderPass(
      const render::backend::FrameBuffer *frame_buffer
  ) override;

  virtual void endRenderPass() override;

  // bind
  void bindUniformBuffer(
      uint32_t unit,
      const render::backend::UniformBuffer *uniform_buffer
  ) override;

  void bindTexture(
      uint32_t unit,
      const render::backend::Texture *texture
  ) override;

  void bindShader(
      const render::backend::Shader *shader
  ) override;

  // draw
  void drawIndexedPrimitive(
      const render::backend::RenderPrimitive *render_primitive
  ) override;

  void drawIndexedPrimitiveInstanced(
      const render::backend::RenderPrimitive *primitive,
      const render::backend::VertexBuffer *instance_buffer,
      uint32_t num_instances,
      uint32_t offset
  ) override;
};
}

#endif //GAMEENGINE_API_H
