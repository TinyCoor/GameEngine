//
// Created by y123456 on 2021/10/11.
//

#ifndef GAMEENGINE_VULKANAPPLICATION_H
#define GAMEENGINE_VULKANAPPLICATION_H
#include "VulkanRender.h"
#include "volk.h"
#include <vector>

class GLFWwindow;

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
    ~Application();
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
    void shutdownVulkan();

    void initVulkanSwapChain();
    void shutdownSwapChain();

    void initScene();
    void shutdownScene();

    void initRender();
    void shutdownRender();

    void mainLoop();
    void RenderFrame();
    void shutdownWindow();

    void recreateSwapChain();

    static void OnFrameBufferResized(GLFWwindow* window,int width,int height);

private:
    VkFormat selectOptimalSupportedFormat(
            const std::vector<VkFormat>& candiates,
            VkImageTiling tiling,
            VkFormatFeatureFlags features);

    VkFormat selectOptimalDepthFormat();

private:
    GLFWwindow* window{nullptr};
    VulkanRenderScene* scene{nullptr};
    VulkanRender* render{nullptr};

    bool frameBufferResized{false};

    VulkanRenderContext context;

    VkInstance instance{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkSurfaceKHR surface {VK_NULL_HANDLE};

    VkQueue graphicsQueue {VK_NULL_HANDLE};
    VkQueue presentQueue {VK_NULL_HANDLE};


    VkCommandPool commandPool =VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};

    VkImage colorImage;
    VkImageView colorImageView;
    VkDeviceMemory colorImageMemory;


    //SwapChain
    VkImage depthImage;
    VkImageView depthImageView;
    VkDeviceMemory depthImageMemory;
    VkFormat depthFormat;
    VkFormat swapChainImageFormat; //color format
    VkExtent2D swapChainExtent;
    VkDescriptorPool descriptorPool=VK_NULL_HANDLE;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkImage> swapChainImages;
    VkSwapchainKHR  swapchain{VK_NULL_HANDLE};


    enum {
        MAX_FRAME_IN_FLIGHT =2,
    };
    size_t currentFrame =0;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    static std::vector<const char*> requiredPhysicalDeviceExtensions;
    static std::vector<const char*> requiredValidationLayers;
};


#endif //GAMEENGINE_VULKANAPPLICATION_H
