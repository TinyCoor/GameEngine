//
// Created by y123456 on 2021/10/13.
//

#ifndef GAMEENGINE_VULKAN_UTILS_H
#define GAMEENGINE_VULKAN_UTILS_H
#include<vulkan.h>
#include<vector>
#include <string>

struct QueueFamilyIndices{
    std::pair<bool,uint32_t> graphicsFamily;
    std::pair<bool,uint32_t> presentFamily{std::make_pair(false,0)};

    bool isComplete()const{
        return graphicsFamily.first && presentFamily.first;
    }
};


struct RenderContext;


class vulkanUtils{

public:
    static void createBuffer( const RenderContext& context,
                              VkDeviceSize size,
                              VkBufferUsageFlags  usageFlags,
                              VkMemoryPropertyFlags  memoryFlags,
                              VkBuffer& buffer,
                              VkDeviceMemory& memory);


   static uint32_t findMemoryType(const RenderContext& context,
                            uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

   static void copyBuffer(const RenderContext& context,
                   VkBuffer srcBuffer,
                   VkBuffer dstBuffer,
                   VkDeviceSize size);

   static void copyBufferToImage(const RenderContext& context,
                           VkBuffer srcBuffer,
                           VkImage dstBuffer,
                           uint32_t width,
                           uint32_t height);

   static void createImage2D(
           const RenderContext& context,
           uint32_t width,
           uint32_t height,
           VkFormat format,
           VkImageTiling tiling,
           VkImageUsageFlags usage,
           VkMemoryPropertyFlags properties,
           VkImage& image,
           VkDeviceMemory& memory
           );

   static void endSingleTimeCommands(const RenderContext& context,VkCommandBuffer commandBuffer);

   static VkCommandBuffer beginSingleTimeCommands(const RenderContext& context);

   static std::vector<char> readFile(const std::string& filename);

   static VkImageView createImage2DVIew(const RenderContext& context,
                                        VkImage image,
                                        VkFormat format);

   static void transitionImageLayout(const RenderContext& context,
                                     VkImage image,
                                     VkFormat format,
                                     VkImageLayout oldLayout,
                                     VkImageLayout newLayout);

   static VkSampler createSampler2D(const RenderContext& context);

};


#endif //GAMEENGINE_VULKAN_UTILS_H
