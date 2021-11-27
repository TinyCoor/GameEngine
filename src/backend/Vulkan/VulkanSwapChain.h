//
// Created by y123456 on 2021/10/11.
//

#ifndef GAMEENGINE_VULKANSWAPCHAIN_H
#define GAMEENGINE_VULKANSWAPCHAIN_H
#include "VulkanContext.h"
#include <volk.h>
#include <vector>

namespace render::backend::vulkan {

struct VulkanRenderFrame {

  VkDescriptorSet swapchainDescriptorSet{VK_NULL_HANDLE};

  VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
  VkFramebuffer frameBuffer{VK_NULL_HANDLE};

  VkBuffer uniformBuffers{VK_NULL_HANDLE};
  VkDeviceMemory uniformBuffersMemory{VK_NULL_HANDLE};

};

struct RenderState;

class VulkanSwapChain {
public:
  VulkanSwapChain(const VulkanContext *ctx, void *nativeWindow, VkDeviceSize uboSize);
  virtual ~VulkanSwapChain();

  void init(int width, int height);

  bool Acquire(const RenderState &state, VulkanRenderFrame &frame);

  bool Present(VulkanRenderFrame &frame);

  void reinit(int width, int height);

  void shutdown();

  inline VkExtent2D getExtent() const { return swapChainExtent; }
  inline VkRenderPass getRenderPass() const { return renderPass; }
  inline VkRenderPass getNoClearRenderPass() const { return noClearRenderPass; }
  inline VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
  inline uint32_t getNumImages() const { return swapChainImageViews.size(); }

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

  VulkanSwapChain::SupportDetails fetchSwapChainSupportDetails(VkPhysicalDevice physicalDevice,
                                                               VkSurfaceKHR surface) const;
  VulkanSwapChain::Settings selectOptimalSwapChainSettings(const VulkanSwapChain::SupportDetails &details) const;

private:

  void initTransient(int width, int height);
  void shutdownTransient();

  void initPersistent();
  void shutdownPersistent();

  void initFrames(VkDeviceSize uboSize);
  void shutdownFrames();
private:

  const VulkanContext *context;
  VkRenderPass renderPass{VK_NULL_HANDLE};
  VkRenderPass noClearRenderPass{VK_NULL_HANDLE};
  VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};

  void *nativeWindow{nullptr};
  VkSurfaceKHR surface{VK_NULL_HANDLE};
  VkSwapchainKHR swapchain{VK_NULL_HANDLE};

  uint32_t presentQueueFamily{0xFFFF};
  VkQueue presentQueue{VK_NULL_HANDLE};

  VulkanSwapChain::SupportDetails details;
  VulkanSwapChain::Settings settings;

  //
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  VkFormat swapChainImageFormat; //color format
  VkFormat depthFormat;

  VkExtent2D swapChainExtent;
  VkDeviceSize uboSize;

  VkImage colorImage;
  VkImageView colorImageView;
  VkDeviceMemory colorImageMemory;

  VkImage depthImage;
  VkImageView depthImageView;
  VkDeviceMemory depthImageMemory;

  //
  std::vector<VulkanRenderFrame> frames;

  //SwapChain
  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  enum {
    MAX_FRAME_IN_FLIGHT = 2,
  };

  uint32_t imageIndex = 0;
  size_t currentFrame = 0;

};

}
#endif //GAMEENGINE_VULKANAPPLICATION_H
