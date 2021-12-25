//
// Created by 12132 on 2021/11/7.
//

#ifndef GAMEENGINE_VULKANIMGUIRENDER_H
#define GAMEENGINE_VULKANIMGUIRENDER_H

#include <volk.h>
#include "driver.h"
struct ImGuiContext;

namespace render::backend::vulkan {

struct RenderState;

class VulkanRenderScene;
struct VulkanRenderFrame;
class VulkanSwapChain;
class VulkanTexture;
class Device;

class ImGuiRender {
public:
  ImGuiRender(render::backend::Driver *driver,
                             ImGuiContext *imgui_ctx,
                             VkExtent2D extent,
                             VkRenderPass renderPass);

  virtual ~ImGuiRender();

  void init(VulkanSwapChain* swapChain);

  void shutdown();

  void resize(VulkanSwapChain* swapChain);

  void render(const VulkanRenderFrame &frame);

private:
  render::backend::Driver *driver{nullptr};
  ImGuiContext *imGuiContext{nullptr};
  VkRenderPass renderPass;
  VkExtent2D extent;
};

}
#endif //GAMEENGINE_VULKANIMGUIRENDER_H
