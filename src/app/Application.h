//
// Created by y123456 on 2021/10/11.
//

#ifndef GAMEENGINE_APPLICATION_H
#define GAMEENGINE_APPLICATION_H

#include "Render.h"
#include <vulkan.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/vec4.hpp>
#include <stdexcept>
#include <iostream>
#include <optional>
#include <vector>
#include <set>
#include <limits>
#include <algorithm>

struct SwapchainSupportedDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct SwapchainSettings{
    VkSurfaceFormatKHR format;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;
};

class Application{
public:
    void run();
private:
    bool checkValidationLayers(std::vector<const char*>& layers);
    bool checkRequiredExtension(std::vector<const char*>& extensions);
    bool checkPhysicalDevice(VkPhysicalDevice physical_device,VkSurfaceKHR& v_surface);
    bool checkRequiredPhysicalDeviceExtensions(VkPhysicalDevice device,
                                               std::vector<const char*>& extensions);
    QueueFamilyIndices fetchFamilyIndices(VkPhysicalDevice physical_device);
    SwapchainSupportedDetails fetchSwapchainSupportedDetails(VkPhysicalDevice& physical_device,
                                                             VkSurfaceKHR& surface);
    SwapchainSettings selectOptimalSwapchainSettings(SwapchainSupportedDetails& details);

    void initWindow();
    void initVulkan();
    void initVulkanExtensions();
    void shutdownVulkan();
    void mainLoop();
    void RenderFrame();
    void shutdownWindow();

    void initRender();
    void shutdownRender();

private:
    GLFWwindow* window{nullptr};

    Render* render{nullptr};

    VkInstance instance{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphicsQueue {VK_NULL_HANDLE};
    VkQueue presentQueue {VK_NULL_HANDLE};
    VkSurfaceKHR surface {VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};
    VkSwapchainKHR  swapchain{VK_NULL_HANDLE};
    VkCommandPool commandPool =VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    enum {
        MAX_FRAME_IN_FLIGHT =2,
    };
    size_t currentFrame =0;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    static std::vector<const char*> requiredPhysicalDeviceExtensions;
    static std::vector<const char*> requiredValidationLayers;
    static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugMessenger;
    static PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugMessenger;
};


#endif //GAMEENGINE_APPLICATION_H
