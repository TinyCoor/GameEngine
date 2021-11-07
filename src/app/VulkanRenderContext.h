//
// Created by y123456 on 2021/10/15.
//

#ifndef GAMEENGINE_VULKANRENDERCONTEXT_H
#define GAMEENGINE_VULKANRENDERCONTEXT_H
#include <volk.h>
#include <vector>

struct VulkanRenderContext{
    VkInstance  instance{VK_NULL_HANDLE};
    VkSurfaceKHR surface= VK_NULL_HANDLE;
    VkDevice device_ =VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkCommandPool commandPool =VK_NULL_HANDLE;
    uint32_t graphicsQueueFamily{0};
    uint32_t presentQueueFamily{0};
    VkQueue graphicsQueue =VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkSampleCountFlagBits maxMSAASamples;
    VkDescriptorPool descriptorPool=VK_NULL_HANDLE;
};

struct VulkanSwapChainContext{
    VkFormat colorFormat;
    VkFormat depthFormat ;
    VkExtent2D extend;

    std::vector<VkImageView> imageViews;
    VkImageView depthImageView =VK_NULL_HANDLE;
    VkImageView colorImageView;

};

#endif //GAMEENGINE_VULKANRENDERCONTEXT_H
