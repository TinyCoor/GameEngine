//
// Created by y123456 on 2021/10/15.
//

#ifndef GAMEENGINE_VULKANCONTEXT_H
#define GAMEENGINE_VULKANCONTEXT_H
#include <volk.h>
#include <vector>
#include <cassert>
class GLFWwindow;


//Singleton
namespace render::backend::vulkan {
class VulkanContext {

public:
  VulkanContext();
  ~VulkanContext();

  void init();
  void shutdown();
  void wait();

  inline const VkInstance &Instance() const { return instance; }
  inline const VkPhysicalDevice &PhysicalDevice() const { return physicalDevice; }
  inline const VkDevice &Device() const { return device; }
  inline const VkCommandPool &CommandPool() const { return commandPool; }
  inline const uint32_t GraphicsQueueFamily() const { return graphicsQueueFamily; }
  inline const VkQueue &GraphicsQueue() const { return graphicsQueue; }
  inline const VkDescriptorPool &DescriptorPool() const { return descriptorPool; }
  inline const VkSampleCountFlagBits MaxMSAASamples() const { return maxMSAASamples; }
private:

  int checkPhysicalDevice(VkPhysicalDevice physical_device);
  VkPhysicalDevice PickPhysicalDevice(VkInstance instance);

private:

  VkInstance instance{VK_NULL_HANDLE};
  VkDevice device = VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkCommandPool commandPool = VK_NULL_HANDLE;
  VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
  uint32_t graphicsQueueFamily{0xFFFF};
  VkQueue graphicsQueue = VK_NULL_HANDLE;
  VkSampleCountFlagBits maxMSAASamples{VK_SAMPLE_COUNT_1_BIT};
  VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};

  /// 同一物理设备下有多个队列，每个队列可能支持的功能不一致

  /// 图形 和 呈现 需要找到支持这两个设备的队列索引(族)和队列

};

}


#endif //GAMEENGINE_VULKANCONTEXT_H
