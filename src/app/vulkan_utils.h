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


struct RenderContext{
    VkDevice device_ =VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkExtent2D extend;
    VkCommandPool commandPool =VK_NULL_HANDLE;
    VkFormat format ;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    std::vector<VkImageView> imageViews;
};


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

    static std::vector<char> readFile(const std::string& filename);
};


#endif //GAMEENGINE_VULKAN_UTILS_H
