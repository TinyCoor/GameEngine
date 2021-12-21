//
// Created by 12132 on 2021/11/9.
//

#ifndef GAMEENGINE_VULKANTEXTURE2DRENDER_H
#define GAMEENGINE_VULKANTEXTURE2DRENDER_H
#include <memory>
#include <volk.h>

#include "driver.h"
#include "VulkanMesh.h"
namespace render::backend::vulkan {

class Device;
class VulkanTexture;
class VulkanShader;

class VulkanTexture2DRender {
public:

  VulkanTexture2DRender(render::backend::Driver *driver);

  void init(VulkanShader &vertex_shader,
            VulkanShader &fragment_shader,
            VulkanTexture &target_texture);

  void shutdown();

  void render();

private:
  const Device *context{nullptr};
  render::backend::Driver *driver {nullptr};
  render::backend::FrameBuffer *framebuffer {nullptr};
  VulkanMesh quad;
  VkExtent2D target_extent;

  VkPipeline pipeline{VK_NULL_HANDLE};
  VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};
  VkRenderPass render_pass{VK_NULL_HANDLE};

  VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
  VkFence fence{VK_NULL_HANDLE};

};
}

#endif //GAMEENGINE_VULKANTEXTURE2DRENDER_H
