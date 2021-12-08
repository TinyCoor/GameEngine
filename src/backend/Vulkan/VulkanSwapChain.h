//
// Created by y123456 on 2021/10/11.
//

#ifndef GAMEENGINE_VULKANSWAPCHAIN_H
#define GAMEENGINE_VULKANSWAPCHAIN_H
#include "driver.h"
#include <volk.h>
#include <vector>

namespace render::backend::vulkan {

struct VulkanRenderFrame {

  VkDescriptorSet descriptor_set{VK_NULL_HANDLE};
  VkCommandBuffer command_buffer{VK_NULL_HANDLE};

  render::backend::FrameBuffer *frame_buffer {nullptr};
  render::backend::UniformBuffer *uniform_buffer {nullptr};
  void* uniform_buffer_data{nullptr};

};

struct RenderState;

class VulkanSwapChain {
public:
  VulkanSwapChain(render::backend::Driver* driver, void *nativeWindow, VkDeviceSize ubo_size);
  virtual ~VulkanSwapChain();

  void init(int width, int height);

  bool Acquire(void* state, VulkanRenderFrame &frame);

  bool Present(VulkanRenderFrame &frame);

  void reinit(int width, int height);

  void shutdown();

  VkExtent2D getExtent() const ;
  uint32_t getNumImages() const ;
  inline VkRenderPass getRenderPass() const { return render_pass; }
  inline VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }


private:

  struct SupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  struct Settings {
    VkSurfaceFormatKHR format;
    VkPresentModeKHR presentMode;
  };

private:
  void beginFrame(void *state, const VulkanRenderFrame &frame);
  void endFrame(const VulkanRenderFrame &frame);

  void initTransient(int width, int height,VkFormat image_format);
  void shutdownTransient();

  void initPersistent(VkFormat image_format);
  void shutdownPersistent();

  void initFrames(VkDeviceSize uboSize,uint32_t width,uint32_t height,uint32_t num_images);
  void shutdownFrames();
private:

  const VulkanContext *context{nullptr};
  render::backend::Driver* driver{nullptr};
  render::backend::SwapChain* swap_chain{nullptr};

  void* native_window{nullptr};

  VkRenderPass render_pass{VK_NULL_HANDLE};
  VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};

  VkDeviceSize uboSize;

  render::backend::Texture *color {nullptr};
  render::backend::Texture *depth {nullptr};
  render::backend::Format depth_format {render::backend::Format::UNDEFINED};

  std::vector<VulkanRenderFrame> frames;

};

}
#endif //GAMEENGINE_VULKANAPPLICATION_H
