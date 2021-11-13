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

class VulkanContext{

public:
    VulkanContext();
    ~VulkanContext();

    VkInstance  instance{VK_NULL_HANDLE};
    VkSurfaceKHR surface= VK_NULL_HANDLE;

    VkDevice device =VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    VkCommandPool commandPool =VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool=VK_NULL_HANDLE;

    uint32_t graphicsQueueFamily{0};
    uint32_t presentQueueFamily{0};
    VkQueue graphicsQueue =VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    VkSampleCountFlagBits maxMSAASamples{VK_SAMPLE_COUNT_1_BIT};

    void init(GLFWwindow* window);
    void shutdown();

private:

    int checkPhysicalDevice(VkPhysicalDevice physical_device,VkSurfaceKHR& v_surface);
    VkPhysicalDevice PickPhysicalDevice(VkInstance instance,VkSurfaceKHR surface);

private:

    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};


};




#endif //GAMEENGINE_VULKANCONTEXT_H
