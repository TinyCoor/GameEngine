//
// Created by y123456 on 2021/10/13.
//

#ifndef GAMEENGINE_VULKANUTILS_H
#define GAMEENGINE_VULKANUTILS_H
#include"volk.h"
#include<vector>
#include <string>

struct QueueFamilyIndices{
    std::pair<bool,uint32_t> graphicsFamily;
    std::pair<bool,uint32_t> presentFamily{std::make_pair(false,0)};
    bool isComplete()const{
        return graphicsFamily.first && presentFamily.first;
    }
};

struct VulkanRenderContext;

class vulkanUtils{

public:
    static void createBuffer( const VulkanRenderContext& context,
                              VkDeviceSize size,
                              VkBufferUsageFlags  usageFlags,
                              VkMemoryPropertyFlags  memoryFlags,
                              VkBuffer& buffer,
                              VkDeviceMemory& memory);


   static uint32_t findMemoryType(const VulkanRenderContext& context,
                            uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

   static void copyBuffer(const VulkanRenderContext& context,
                   VkBuffer srcBuffer,
                   VkBuffer dstBuffer,
                   VkDeviceSize size);

   static void copyBufferToImage(const VulkanRenderContext& context,
                           VkBuffer srcBuffer,
                           VkImage dstBuffer,
                           uint32_t width,
                           uint32_t height);

   static void createImage2D(
           const VulkanRenderContext& context,
           uint32_t width,
           uint32_t height,
           uint32_t mipLevel,
           VkSampleCountFlagBits numberSample,
           VkFormat format,
           VkImageTiling tiling,
           VkImageUsageFlags usage,
           VkMemoryPropertyFlags properties,
           VkImage& image,
           VkDeviceMemory& memory
           );

   static VkImageView createImage2DView(const VulkanRenderContext& context,
                                         VkImage image,
                                         uint32_t minLevels,
                                         VkFormat format,
                                         VkImageAspectFlags aspectFlags
                                         );

   static void endSingleTimeCommands(const VulkanRenderContext& context,VkCommandBuffer commandBuffer);

   static VkCommandBuffer beginSingleTimeCommands(const VulkanRenderContext& context);

   static std::vector<char> readFile(const std::string& filename);

   static void transitionImageLayout(const VulkanRenderContext& context,
                                     VkImage image,
                                     uint32_t mipLevels,
                                     VkFormat format,
                                     VkImageLayout oldLayout,
                                     VkImageLayout newLayout);

   static VkSampler createSampler2D(const VulkanRenderContext& context, uint32_t mipLevels);

   static bool hasStencilComponent(VkFormat format);

   static void generateImage2DMipMaps(
           const VulkanRenderContext& context,
           VkImage image,
           uint32_t width,
           uint32_t height,
           uint32_t mipLevel,
           VkFormat format,
           VkFilter filter
           );

   static VkSampleCountFlagBits getMaxUsableSampleCount(const VulkanRenderContext& context);

   static VkShaderModule createShaderModule(const VulkanRenderContext& context,
                                      uint32_t * code,
                                      uint32_t size);

    static void bindUniformBuffer(
            const VulkanRenderContext &context,
            VkDescriptorSet descriptorSet,
            int binding,
            VkBuffer buffer,
            VkDeviceSize offset,
            VkDeviceSize size
    );

    static void bindCombinedImageSampler(
            const VulkanRenderContext &context,
            VkDescriptorSet descriptorSet,
            int binding,
            VkImageView imageView,
            VkSampler sampler
    );

};


#endif //GAMEENGINE_VULKANUTILS_H
