//
// Created by y123456 on 2021/10/10.
//

#pragma once

#include "Device.h"
#include "driver.h"
#include "VulkanMesh.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"
namespace render::backend::vulkan {
class VulkanCubeMapRender {
private:
  render::backend::Driver *driver {nullptr};
  VulkanMesh quad;

  render::backend::BindSet* bind_set{nullptr};
  render::backend::CommandBuffer* command_buffer{nullptr};
  render::backend::FrameBuffer *framebuffer {nullptr};
  render::backend::UniformBuffer *uniform_buffer {nullptr};

public:
  VulkanCubeMapRender(render::backend::Driver *driver);

  void init(VulkanTexture& target_texture,int target_mip);

  void shutdown();

  void render(const VulkanShader& vertShader,
              const VulkanShader& fragShader,
              const VulkanTexture& inputTexture,
              int input_mip = -1,
              uint8_t size =0,
              const void* data = nullptr);
};
}