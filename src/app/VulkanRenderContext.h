//
// Created by y123456 on 2021/10/15.
//

#ifndef GAMEENGINE_VULKANRENDERCONTEXT_H
#define GAMEENGINE_VULKANRENDERCONTEXT_H
#include <vulkan.h>
#include <vector>

struct RenderContext{
    VkDevice device_ =VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkExtent2D extend;
    VkCommandPool commandPool =VK_NULL_HANDLE;
    VkFormat colorFormat;
    VkFormat depthFormat ;
    VkDescriptorPool descriptorPool=VK_NULL_HANDLE;
    VkQueue graphicsQueue =VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    std::vector<VkImageView> imageViews;
    VkImageView depthImageView =VK_NULL_HANDLE;
};


#endif //GAMEENGINE_VULKANRENDERCONTEXT_H
