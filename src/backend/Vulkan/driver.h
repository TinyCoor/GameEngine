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
  VkImageView view {VK_NULL_HANDLE};
  VkSampler sampler {VK_NULL_HANDLE};
  VkDeviceMemory imageMemory{VK_NULL_HANDLE};
  VkImageType type{VK_IMAGE_TYPE_2D};
  VkFormat format{VK_FORMAT_R8G8B8A8_UNORM};
  size_t width{0};
  size_t height{0};
  size_t depth{0};
  uint8_t num_layers{0};
  uint8_t num_mipmaps{0};
  VkImageTiling tiling{VK_IMAGE_TILING_OPTIMAL};
  VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};
  VkImageCreateFlags flags{0};
};


struct FrameBufferColorAttachment : public render::backend::FrameBufferColorAttachment
{
  VkImageView view {VK_NULL_HANDLE};
};

struct FrameBufferDepthStencilAttachment : public render::backend::FrameBufferColorAttachment
{
  VkImageView view {VK_NULL_HANDLE};
};

struct FrameBuffer : public render::backend::FrameBuffer {
  static constexpr int MAX_COLOR_ATTACHMENTS = 16;
  VkRenderPass dummy_render_pass{VK_NULL_HANDLE};
  VkFramebuffer framebuffer{VK_NULL_HANDLE};
  uint8_t num_color_attachments{0};
  FrameBufferColorAttachment color_attachments[MAX_COLOR_ATTACHMENTS];
  FrameBufferDepthStencilAttachment depthstencil_attachment;
};


struct UniformBuffer : public render::backend::UniformBuffer {
  VkBuffer buffer{VK_NULL_HANDLE};
  VkDeviceMemory memory{VK_NULL_HANDLE};
  uint32_t size{0};
  void* pointer {nullptr};
};


struct Shader : public render::backend::Shader {
  ShaderType type{ShaderType::FRAGMENT};
  VkShaderModule shaderModule{VK_NULL_HANDLE};

};

struct GraphicsProgram : public render::backend::GraphicsProgram {
  const Shader *vertex;
  const Shader *tessellation_control;
  const Shader *tessellation_evaulation;
  const Shader *geometry;
  const Shader *fragment;
};

struct SwapChain : public render::backend::SwapChain
{
  enum {
    MAX_IMAGES =8,
    MAX_FENCES_IN_FLIGHT = 4,
  };

  VkSurfaceKHR surface{VK_NULL_HANDLE};
  VkSurfaceCapabilitiesKHR surface_capabilities;
  VkSurfaceFormatKHR surface_format;

  uint32_t present_queue_family{0XFFFF};
  VkQueue present_queue{VK_NULL_HANDLE};
  VkPresentModeKHR present_mode{VK_PRESENT_MODE_FIFO_KHR};

  VkFormat format{VK_FORMAT_UNDEFINED};
  VkExtent2D sizes;



  VkSemaphore image_available_gpu[vulkan::SwapChain::MAX_IMAGES];
  VkSemaphore render_finished_gpu[vulkan::SwapChain::MAX_IMAGES];
  VkFence rendering_finished_cpu[vulkan::SwapChain::MAX_IMAGES];

  VkSwapchainKHR swapchain{VK_NULL_HANDLE};
  uint32_t num_images{0};
  uint32_t current_image{0};


  VkImage images[vulkan::SwapChain::MAX_IMAGES];
  VkImageView views[vulkan::SwapChain::MAX_IMAGES];
  VkCommandBuffer commandBuffer[vulkan::SwapChain::MAX_IMAGES];


};


class VulkanContext;
class VulkanDriver : public render::backend::Driver {
  VulkanContext* context {nullptr};
public:
  VulkanDriver(const char* app_name,const char* engine_name);
  virtual ~VulkanDriver() =default;

  VulkanContext* GetVulkanContext() const {return context;}

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
      IndexSize index_size,
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
      const render::backend::FrameBufferColorAttachment *color_attachments,
      const render::backend::FrameBufferDepthStencilAttachment *depthstencil_attachment = nullptr
  ) override;

  UniformBuffer *createUniformBuffer(
      BufferType type,
      uint32_t size,
      const void *data = nullptr
  ) override;

  Shader *createShaderFromSource(
      ShaderType type,
      uint32_t length,
      const char *data,
      const char *path
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
  void generateTexture2DMipmaps(render::backend::Texture *texture) override;

  void* map(render::backend::UniformBuffer* uniform_buffer) override;

  void unmap(render::backend::UniformBuffer* uniform_buffer) override;

  void wait();

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

  SwapChain *createSwapChain(void *native_window) override;
  void destroySwapChain(render::backend::SwapChain *swapchain) override;
  bool acquire(render::backend::SwapChain *swapchain) override;
  bool present(render::backend::SwapChain *swapchain) override;
  bool resize(render::backend::SwapChain *swapchain, uint32_t width, uint32_t height) override;
};
}

#endif //GAMEENGINE_API_H
