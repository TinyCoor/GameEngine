//
// Created by 12132 on 2021/11/7.
//

#ifndef GAMEENGINE_VULKANIMGUIRENDER_H
#define GAMEENGINE_VULKANIMGUIRENDER_H

#include <memory>
#include <volk.h>
struct ImGuiContext;


namespace render::backend::vulkan {

struct RenderState;

class VulkanRenderScene;
struct VulkanRenderFrame;
class VulkanSwapChain;
class VulkanTexture;
class VulkanContext;

class ImGuiRender {
public:
  ImGuiRender(const VulkanContext *ctx,
                             ImGuiContext *imgui_ctx,
                             VkExtent2D extent,
                             VkRenderPass renderPass);

  virtual ~ImGuiRender();

  void init(VulkanSwapChain* swapChain);

  void shutdown();

  void resize(VulkanSwapChain* swapChain);

  void render(const VulkanRenderFrame &frame);

private:
  const VulkanContext *context{nullptr};
  ImGuiContext *imGuiContext{nullptr};
  VkRenderPass renderPass;
  VkExtent2D extent;
};

}
#endif //GAMEENGINE_VULKANIMGUIRENDER_H
