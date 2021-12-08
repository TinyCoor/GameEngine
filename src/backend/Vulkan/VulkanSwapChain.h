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

  VkDescriptorSet swapchainDescriptorSet{VK_NULL_HANDLE};

  VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
  VkFramebuffer frameBuffer{VK_NULL_HANDLE};

  void* uniformBufferData{nullptr};
  VkBuffer uniformBuffers{VK_NULL_HANDLE};
  VkDeviceMemory uniformBuffersMemory{VK_NULL_HANDLE};

};

struct RenderState;

class VulkanSwapChain {
public:
  VulkanSwapChain(render::backend::Driver* driver, void *nativeWindow, VkDeviceSize uboSize);
  virtual ~VulkanSwapChain();

  void init(int width, int height);

  bool Acquire(const RenderState &state, VulkanRenderFrame &frame);

  bool Present(VulkanRenderFrame &frame);

  void reinit(int width, int height);

  void shutdown();

  VkExtent2D getExtent() const ;
  uint32_t getNumImages() const ;
  inline VkRenderPass getRenderPass() const { return renderPass; }
  inline VkRenderPass getNoClearRenderPass() const { return noClearRenderPass; }
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

  void initTransient(int width, int height,VkFormat image_format);
  void shutdownTransient();

  void initPersistent(VkFormat image_format);
  void shutdownPersistent();

  void initFrames(VkDeviceSize uboSize,uint32_t width,uint32_t height,uint32_t num_images,VkImageView* views);
  void shutdownFrames();
private:

  const VulkanContext *context{nullptr};
  render::backend::Driver* driver{nullptr};
  render::backend::SwapChain* swapChain{nullptr};

  void* native_window{nullptr};

  VkRenderPass renderPass{VK_NULL_HANDLE};
  VkRenderPass noClearRenderPass{VK_NULL_HANDLE};
  VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};

  VkDeviceSize uboSize;


  VkImage colorImage;
  VkImageView colorImageView;
  VkDeviceMemory colorImageMemory;

  VkImage depthImage;
  VkFormat depthFormat;
  VkImageView depthImageView;
  VkDeviceMemory depthImageMemory;

  //
  std::vector<VulkanRenderFrame> frames;

};

}
#endif //GAMEENGINE_VULKANAPPLICATION_H
