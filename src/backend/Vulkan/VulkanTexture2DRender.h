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

  ~VulkanTexture2DRender();

  void init(const VulkanTexture* target_texture);

  void shutdown();

  void render(const VulkanShader* vertex_shader,const VulkanShader* fragment_shader);

private:
  render::backend::Driver *driver {nullptr};
  render::backend::FrameBuffer *framebuffer {nullptr};
  render::backend::CommandBuffer* command_buffer{nullptr};
  VulkanMesh quad;

};
}

#endif //GAMEENGINE_VULKANTEXTURE2DRENDER_H
