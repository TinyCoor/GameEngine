//
// Created by y123456 on 2021/10/13.
//

#ifndef GAMEENGINE_VULKANUTILS_H
#define GAMEENGINE_VULKANUTILS_H

#include <volk.h>
#include <vector>
#include <string>
#include <optional>

namespace render::backend::vulkan {
struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily{std::nullopt};
  std::optional<uint32_t> presentFamily{std::nullopt};
  inline bool isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

struct VulkanContext;

class VulkanUtils {

public:

  static bool checkInstanceValidationLayers(
      const std::vector<const char *> &requiredLayers,
      bool verbose = false
  );

  static bool checkInstanceExtensions(
      const std::vector<const char *> &requiredExtensions,
      bool verbose = false
  );

  static bool checkPhysicalDeviceExtensions(
      VkPhysicalDevice physicalDevice,
      const std::vector<const char *> &requiredExtensions,
      bool verbose = false
  );

  static uint32_t fetchGraphicsQueueFamily(
      VkPhysicalDevice physicalDevice
  );

  static uint32_t fetchPresentQueueFamily(
      VkPhysicalDevice physicalDevice,
      VkSurfaceKHR surface,
      uint32_t graphicsQueueFamily
  );

  static VkFormat selectOptimalSupportedImageFormat(const VkPhysicalDevice &physicalDevice,
                                                    const std::vector<VkFormat> &candiates,
                                                    VkImageTiling tiling,
                                                    VkFormatFeatureFlags features);

  static VkFormat selectOptimalImageFormat(const VkPhysicalDevice &physicalDevice);

  static void createBuffer(const VulkanContext *context,
                           VkDeviceSize size,
                           VkBufferUsageFlags usageFlags,
                           VkMemoryPropertyFlags memoryFlags,
                           VkBuffer &buffer,
                           VkDeviceMemory &memory);

  static void createDeviceLocalBuffer(const VulkanContext *context,
                                 VkDeviceSize size,
                                 const void* data,
                                 VkBufferUsageFlags usageFlags,
                                 VkBuffer &buffer,
                                 VkDeviceMemory &memory);


  static uint32_t findMemoryType(const VkPhysicalDevice physicalDevice,
                                 uint32_t typeFilter,
                                 VkMemoryPropertyFlags properties);

  static void copyBuffer(const VulkanContext *context,
                         VkBuffer srcBuffer,
                         VkBuffer dstBuffer,
                         VkDeviceSize size);

  static void copyBufferToImage(const VulkanContext *context,
                                VkBuffer src,
                                VkImage dst,
                                uint32_t width,
                                uint32_t height);

  static void copyBufferToImage(const VulkanContext *context,
                                VkBuffer src,
                                VkImage dst,
                                uint32_t width,
                                uint32_t height,
                                uint32_t depth,
                                uint32_t mipLevel,
                                uint32_t layers,
                                VkDeviceSize bufferOffset);

  static void createImage2D(
      const VulkanContext *context,
      uint32_t width,
      uint32_t height,
      uint32_t mipLevel,
      VkSampleCountFlagBits numberSample,
      VkFormat format,
      VkImageTiling tiling,
      VkImageUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkImage &image,
      VkDeviceMemory &memory
  );

  static void createImage(
      const VulkanContext *context,
      VkImageType imageType,
      uint32_t width,
      uint32_t height,
      uint32_t depth,
      uint32_t arrayLayers,
      uint32_t mipLevel,
      VkSampleCountFlagBits numberSample,
      VkFormat format,
      VkImageTiling tiling,
      VkImageUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkImageCreateFlags flags,
      VkImage &image,
      VkDeviceMemory &memory
  );

  static void fillImage(
      const VulkanContext *context, VkImage &image,
      uint32_t width,uint32_t height,uint32_t depth,
      uint32_t mipLevel,uint32_t arrayLayers,uint32_t pixel_size,
      const void* data,VkFormat format,uint32_t dataMipLevel,uint32_t dataLayers
  );

  static void createCubeImage(const VulkanContext *context,
                              uint32_t width,
                              uint32_t height,
                              uint32_t mipLevel,
                              VkSampleCountFlagBits numberSample,
                              VkFormat format,
                              VkImageTiling tiling,
                              VkImageUsageFlags usage,
                              VkMemoryPropertyFlags properties,
                              VkImage &image,
                              VkDeviceMemory &memory);

  static VkImageView createImageCubeView(VkDevice device,
                                         VkImage image,
                                         VkFormat format,
                                         VkImageAspectFlags aspectFlags,
                                         uint32_t baseMipLayer = 0,
                                         uint32_t numMipLevel = 1);

  static VkImageView createImageView(VkDevice device,
                                     VkImage image,
                                     VkFormat format,
                                     VkImageAspectFlags aspectFlags,
                                     VkImageViewType viewType,
                                     uint32_t baseMipLayer = 0,
                                     uint32_t numMipLevel = 1,
                                     uint32_t baseLayer = 0,
                                     uint32_t numLayers = 1);

  static void endSingleTimeCommands(const VulkanContext *context, VkCommandBuffer commandBuffer);

  static VkCommandBuffer beginSingleTimeCommands(const VulkanContext *context);

  static std::vector<char> readFile(const std::string &filename);

  static void transitionImageLayout(const VulkanContext *context,
                                    VkImage image,
                                    VkFormat format,
                                    VkImageLayout oldLayout,
                                    VkImageLayout newLayout,
                                    uint32_t baseMipLayer = 0,
                                    uint32_t numMipLevel = 1,
                                    uint32_t baseLayer = 0,
                                    uint32_t numLayers = 1);

  static VkSampler createSampler(VkDevice device,
                                 int minMipLevel,
                                 int maxMipLevel);


  static void generateImage2DMipMaps(
      const VulkanContext *context,
      VkImage image,
      VkFormat imageFormat,
      uint32_t width,
      uint32_t height,
      uint32_t mipLevel,
      VkFormat format,
      VkFilter filter
  );

  static VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);

  static VkShaderModule createShaderModule(VkDevice device,
                                           const uint32_t *code,
                                           uint32_t size);

  static void bindUniformBuffer(
      VkDevice device,
      VkDescriptorSet descriptorSet,
      int binding,
      VkBuffer buffer,
      VkDeviceSize offset,
      VkDeviceSize size
  );

  static void bindCombinedImageSampler(
      const VkDevice device,
      VkDescriptorSet descriptorSet,
      int binding,
      VkImageView imageView,
      VkSampler sampler
  );

  static QueueFamilyIndices fetchFamilyIndices(VkPhysicalDevice &physcalDevice, VkSurfaceKHR &surface);
 private:
    static bool hasStencilComponent(VkFormat format);
    static bool isDepthFormat(VkFormat format);

};
}
#endif //GAMEENGINE_VULKANUTILS_H
