//
// Created by y123456 on 2021/10/10.
//

#pragma once

#include "VulkanContext.h"
#include "driver.h"
#include "VulkanMesh.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"
#include <memory>
namespace render::backend::vulkan {


class VulkanCubeMapRender {
private:
  const VulkanContext *context{nullptr};
  render::backend::Driver *driver {nullptr};
  VulkanMesh quad;

  VkExtent2D target_extent;

  VkRenderPass render_pass{VK_NULL_HANDLE};
  VkDescriptorSetLayout descriptor_set_layout{VK_NULL_HANDLE};
  VkPipeline pipeline{VK_NULL_HANDLE};
  VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};

  VkCommandBuffer command_buffer{VK_NULL_HANDLE};
  VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
  VkFence fence{VK_NULL_HANDLE};

  render::backend::FrameBuffer *framebuffer {nullptr};
  render::backend::UniformBuffer *uniform_buffer {nullptr};

  uint32_t push_constants_size {0};

public:
  VulkanCubeMapRender(const VulkanContext *context, render::backend::Driver *driver)
      : context(context)
      , driver(driver)
      , quad(driver)
  { }

  void init(VulkanShader& vertShader,
            VulkanShader& fragShader,
            VulkanTexture& target_texture,
            int mip,
            uint32_t userDataSize = 0);

  void shutdown();

  void render(const VulkanTexture& inputTexture,
              float *userData = nullptr, int input_mip = -1);
};
}