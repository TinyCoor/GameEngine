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

class VulkanContext;
class VulkanMesh;
class VulkanTexture;
class VulkanShader;

class VulkanTexture2DRender {
public:
  VulkanTexture2DRender(const VulkanContext *ctx,render::backend::Driver *driver)
  : context(ctx), driver(driver), quad(driver)
  { }

  void init(VulkanShader &vertex_shader,
            VulkanShader &fragment_shader,
            VulkanTexture &target_texture);

  void shutdown();

  void render();

private:
  const VulkanContext *context;
  render::backend::Driver *driver {nullptr};
  render::backend::FrameBuffer *framebuffer {nullptr};
  VulkanMesh quad;
  VkExtent2D target_extent;

  VkPipeline pipeline;
  VkPipelineLayout pipeline_layout;
  VkRenderPass render_pass;

  VkCommandBuffer commandBuffer;
  VkFence fence;



};
}

#endif //GAMEENGINE_VULKANTEXTURE2DRENDER_H
