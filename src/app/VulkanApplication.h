//
// Created by y123456 on 2021/10/11.
//

#ifndef GAMEENGINE_VULKANAPPLICATION_H
#define GAMEENGINE_VULKANAPPLICATION_H
#include "VulkanRenderContext.h"
#include "RenderState.h"
#include <volk.h>
#include <memory>

class GLFWwindow;
class VulkanRender;
class VulkanRenderScene;
class VulkanSwapChain;

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

    void initWindow();
    void initVulkan();
    void shutdownVulkan();

    void initVulkanSwapChain();
    void shutdownSwapChain();

    void initScene();
    void shutdownScene();

    void initImGui();
    void shutdownImGui();

    void initRender();
    void shutdownRender();

    void mainLoop();
    void update();
    void RenderFrame();
    void shutdownWindow();

    void recreateSwapChain();

    static void OnFrameBufferResized(GLFWwindow* window,int width,int height);


private:
    GLFWwindow* window{nullptr};
    VulkanRenderScene* scene{nullptr};
    VulkanRender* render{nullptr};
    RenderState state;

    std::shared_ptr<VulkanSwapChain> swapChain;
    VulkanRenderContext context;

    VkQueue graphicsQueue {VK_NULL_HANDLE};
    VkQueue presentQueue {VK_NULL_HANDLE};


    VkCommandPool commandPool =VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};


    VkDescriptorPool descriptorPool;

    bool windowResized = false;
};


#endif //GAMEENGINE_VULKANAPPLICATION_H
