//
// Created by y123456 on 2021/10/10.
//
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
#include <functional>

#define VK_CHECK(call,msg)                              \
    do {                                                \
        VkResult res  =(call);                            \
        if(res != VK_SUCCESS ){                          \
             throw std::runtime_error(msg);               \
        }                                                 \
    }while(0)

#define TH_WITH_MSG(condition,msg) \
    do{                               \
    if((condition))                  \
        throw std::runtime_error(msg);  \
    }while(0)


#define CERR_MSG(condition,msg) \
    do{                               \
    if((condition))                  \
       std::cout << msg ;       \
       return false;             \
    }while(0)


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}



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

namespace {

    VkApplicationInfo createApplicationInfo(){
        //填充应用信息
        VkApplicationInfo appInfo={};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName ="PBR Render";
        appInfo.applicationVersion= VK_MAKE_VERSION(1,0,0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        return appInfo;
    }

    VkDebugUtilsMessengerCreateInfoEXT createDebugMessengerCreateInfo(
            VkApplicationInfo& appInfo){
        VkDebugUtilsMessengerCreateInfoEXT  debugMessengerInfo={};
        debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                         | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                         | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugMessengerInfo.pfnUserCallback = debugCallback;
        debugMessengerInfo.pUserData = nullptr;
        return debugMessengerInfo;
    }

    VkInstance createInstance(std::vector<const char*>& extensions,
                                            std::vector<const char*>& layers,
                                            VkApplicationInfo& appInfo,
                                            VkDebugUtilsMessengerCreateInfoEXT& debugMessengerInfo ){
        VkInstance  instance;
        VkInstanceCreateInfo instanceInfo={};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo = &appInfo;
        instanceInfo.enabledExtensionCount = extensions.size();
        instanceInfo.ppEnabledExtensionNames =extensions.data();
        instanceInfo.enabledLayerCount = layers.size();
        instanceInfo.ppEnabledLayerNames = layers.data();
        instanceInfo.pNext = &debugMessengerInfo;
        VK_CHECK(vkCreateInstance(&instanceInfo, nullptr,&instance),"Failed to Create Instance\n");
        return instance;
    }

    std::vector<VkDeviceQueueCreateInfo> createDeviceQueueCreateInfo(QueueFamilyIndices& indices){
        static float queuePriority = 1.0;
        std::vector<VkDeviceQueueCreateInfo> queuesInfo;
       // VkDeviceQueueCreateInfo queueCreateInfo ={};
        std::set<uint32_t > uniqueQueueFamilies={ indices.graphicsFamily.second,indices.presentFamily.second};

        for (uintptr_t queueFamilyIndex :uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            info.queueFamilyIndex = queueFamilyIndex;
            info.queueCount = 1;
            info.pQueuePriorities = &queuePriority;
            queuesInfo.push_back(info);
        }
        return queuesInfo;
    }

    VkDevice createDevice(
            VkPhysicalDevice physicalDevice,
            std::vector<VkDeviceQueueCreateInfo>&queuesInfo,
            std::vector<const char*>& requiredPhysicalExtensions,
            std::vector<const char*>& layers)
    {
        VkDevice device;
        VkDeviceCreateInfo deviceCreateInfo={};
        VkPhysicalDeviceFeatures deviceFeatures ={};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = queuesInfo.data();
        deviceCreateInfo.queueCreateInfoCount = queuesInfo.size();
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount =requiredPhysicalExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = requiredPhysicalExtensions.data();
        deviceCreateInfo.enabledLayerCount = layers.size();
        deviceCreateInfo.ppEnabledLayerNames =layers.data();
        VK_CHECK(vkCreateDevice(physicalDevice,&deviceCreateInfo, nullptr,&device),"Create logical Device failed\n");
        return device;
    }


    VkSurfaceKHR createSurface(VkInstance& instance,GLFWwindow* window){
        VkSurfaceKHR  surface;
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.hwnd = glfwGetWin32Window(window);
        surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
        VK_CHECK(vkCreateWin32SurfaceKHR(instance,&surfaceCreateInfo, nullptr,&surface),"Create win32 Surface Error");
        return surface;
    }
}

const int Width =800;
const int Height = 600;

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

    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    static std::vector<const char*> requiredPhysicalDeviceExtensions;
    static std::vector<const char*> requiredValidationLayers;
    static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugMessenger;
    static PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugMessenger;
};

std::vector<const char*> Application:: requiredPhysicalDeviceExtensions ={
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

std::vector<const char*> Application::requiredValidationLayers={
        "VK_LAYER_KHRONOS_validation"
};

PFN_vkCreateDebugUtilsMessengerEXT Application::vkCreateDebugMessenger{VK_NULL_HANDLE};
PFN_vkDestroyDebugUtilsMessengerEXT Application::vkDestroyDebugMessenger{VK_NULL_HANDLE};

void Application::initWindow() {
    glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);

    window = glfwCreateWindow(Width,Height,"Vulkan", nullptr, nullptr);
}

void Application::run(){
    initWindow();
    initVulkan();
    initRender();
    mainLoop();
    shutdownWindow();
    shutdownRender();
    shutdownVulkan();
}

bool Application::checkRequiredExtension(std::vector<const char*>& extensions) {

    uint32_t vulkanExtensionCount =0;
    vkEnumerateInstanceExtensionProperties(nullptr,&vulkanExtensionCount, nullptr);
    std::vector<VkExtensionProperties> vulkanExtensions(vulkanExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr,&vulkanExtensionCount, vulkanExtensions.data());

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> requiredExtensions(glfwExtensions,glfwExtensions + glfwExtensionCount);

    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    extensions.clear();

    for (const auto& requiredExtension : requiredExtensions) {
        bool supported = false;
        for(const auto& ex : vulkanExtensions){
            if(strcmp(requiredExtension,ex.extensionName) ==0){
                supported = true;
                break;
            }
        }
        if(!supported){
            std::cerr << requiredExtension << "is not support this device\n";
            return false;
        }
        std::cout << "Have: "<< requiredExtension << "\n";
        extensions.push_back(requiredExtension);
    }

    return true;
}

bool Application::checkValidationLayers(std::vector<const char *> &layers) {
    uint32_t vulkanLayerCount =0;
    vkEnumerateInstanceLayerProperties(&vulkanLayerCount, nullptr);
    std::vector<VkLayerProperties> vulkanLayers(vulkanLayerCount);
    vkEnumerateInstanceLayerProperties(&vulkanLayerCount, vulkanLayers.data());

    layers.clear();
    for (const auto& requiredLayer : requiredValidationLayers) {
        bool supported = false;
        for(const auto& layer : vulkanLayers){
            if(strcmp(requiredLayer,layer.layerName) ==0){
                supported = true;
                break;
            }
        }
        if(!supported)
            return false;
        std::cout << "Have:" << requiredLayer << "\n";
        layers.push_back(requiredLayer);
    }
    return true;
}

bool Application::checkRequiredPhysicalDeviceExtensions(VkPhysicalDevice physical_device,
                                                        std::vector<const char*>& extensions) {
    uint32_t deviceExtensionCount =0;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr,&deviceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(physical_device,nullptr,&deviceExtensionCount, deviceExtensions.data());

    extensions.clear();
    for(const char* requiredPhysicalDeviceExtension : requiredPhysicalDeviceExtensions){
        bool supported = false;
        for (auto& deviceExtension : deviceExtensions) {
            if(strcmp(requiredPhysicalDeviceExtension,deviceExtension.extensionName) == 0){
                supported = true;
                break;
            }
        }
        if(!supported){
            std::cerr << requiredPhysicalDeviceExtension << " is not supported by this device\n";
            return false;
        }
        std::cout << "Have:" << requiredPhysicalDeviceExtension << "\n";
        extensions.push_back(requiredPhysicalDeviceExtension);
    }
    return true;
}
//TODO fix
bool Application::checkPhysicalDevice(VkPhysicalDevice physical_device,VkSurfaceKHR& v_surface) {

    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,&queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,&queueCount,queueFamilies.data());

    QueueFamilyIndices indices = Application::fetchFamilyIndices(physical_device);
    if(!indices.isComplete())
        return false;

    std::vector<const char*> deviceExtensions;
    if( !checkRequiredPhysicalDeviceExtensions(physical_device,deviceExtensions)){
        return false;
    }
    SwapchainSupportedDetails details = fetchSwapchainSupportedDetails(physical_device,surface);

    if(details.formats.empty() || details.presentModes.empty()){
        return false;
    }

    //TODO only checks for test remove later
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physical_device,&deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physical_device,&deviceFeatures);

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
           && deviceFeatures.geometryShader;
}
void Application::initVulkan() {
    //检查vulkan extension and layer
    std::vector<const char*> extensions;
    TH_WITH_MSG(!checkRequiredExtension(extensions),"This device is not have Vulkan Extension\n");

    std::vector<const char* > layers;
    TH_WITH_MSG(!checkValidationLayers(layers),"This device does not have vulkan Validation layer supported\n");

    auto appInfo =createApplicationInfo();
    auto debugMessengerInfo = createDebugMessengerCreateInfo(appInfo);
    instance = createInstance(extensions,layers,appInfo,debugMessengerInfo);
//
    initVulkanExtensions();

    VK_CHECK( vkCreateDebugMessenger(instance, &debugMessengerInfo, nullptr, &debugMessenger)," CreateDebugUtilsMessengerEXT  Failed\n");
    //create Vulkan Surface TODO cross platform support
    surface = createSurface(instance,window);
    //枚举物理设备
    uint32_t deviceCount =0;
    vkEnumeratePhysicalDevices(instance,&deviceCount, nullptr);
    TH_WITH_MSG(deviceCount == 0,"No Support Vulkan Physical Device\n");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance,&deviceCount, physicalDevices.data());
    for (const auto& physical_device :physicalDevices) {
        if(checkPhysicalDevice(physical_device,surface)){
            physicalDevice= physical_device;
            break;
        }
    }
    TH_WITH_MSG(physicalDevice == VK_NULL_HANDLE,"failed to find GPU\n");

    QueueFamilyIndices indices = fetchFamilyIndices(physicalDevice);
    auto queuesInfo = createDeviceQueueCreateInfo(indices);

   // vkGetPhysicalDeviceFeatures(physical_device,&deviceFeatures);

    device= createDevice(physicalDevice,queuesInfo,this->requiredPhysicalDeviceExtensions,layers);
    vkGetDeviceQueue(device,indices.graphicsFamily.second,0,&graphicsQueue);
    TH_WITH_MSG(graphicsQueue == VK_NULL_HANDLE,
                "Get graphics queue from logical device failed\n");

    vkGetDeviceQueue(device,indices.presentFamily.second,0,&presentQueue);
    TH_WITH_MSG(presentQueue == VK_NULL_HANDLE,
                "Get present queue from logical device failed\n");

    //create SwapChain finally
    SwapchainSupportedDetails details = fetchSwapchainSupportedDetails(physicalDevice,surface);
    SwapchainSettings settings = selectOptimalSwapchainSettings(details);

    uint32_t imageCount = details.capabilities.minImageCount + 1;

    if(details.capabilities.maxImageCount > 0 ){
        imageCount = std::min(imageCount,details.capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR swapChainInfo;
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface =surface;
    swapChainInfo.minImageCount = imageCount;
    swapChainInfo.imageFormat = settings.format.format;
    swapChainInfo.imageColorSpace =settings.format.colorSpace;
    swapChainInfo.imageExtent = settings.extent;
    swapChainInfo.imageArrayLayers= 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    std::vector<uint32_t> familiesQueues ={indices.graphicsFamily.second,indices.presentFamily.second};
    if(indices.graphicsFamily.second != indices.presentFamily.second){
        swapChainInfo.imageSharingMode =VK_SHARING_MODE_CONCURRENT;
        swapChainInfo.queueFamilyIndexCount = 2;
        swapChainInfo.pQueueFamilyIndices = familiesQueues.data();

    } else{
        swapChainInfo.imageSharingMode =VK_SHARING_MODE_EXCLUSIVE;
        swapChainInfo.queueFamilyIndexCount = 0;
        swapChainInfo.pQueueFamilyIndices = nullptr;
    }

    swapChainInfo.preTransform = details.capabilities.currentTransform;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.presentMode = settings.presentMode;
    swapChainInfo.clipped =VK_TRUE;
    swapChainInfo.oldSwapchain = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(device,&swapChainInfo, nullptr,&swapchain),
             "failed to created swapchain\n");

    uint32_t  swapChainImageCount =0;
    vkGetSwapchainImagesKHR(device,swapchain,&swapChainImageCount, nullptr);
    assert(swapChainImageCount > 0 );

    swapChainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(device,swapchain,&swapChainImageCount, swapChainImages.data());

    swapChainImageFormat = settings.format.format;
    swapChainExtent = settings.extent;

    swapChainImageViews.resize(swapChainImageCount);
    for (int i = 0; i <swapChainImageViews.size() ; ++i) {
        VkImageViewCreateInfo imageViewInfo ={};
        imageViewInfo.sType =VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = swapChainImages[i];
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = swapChainImageFormat;

        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.layerCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;

        VK_CHECK(vkCreateImageView(device,&imageViewInfo, nullptr,&swapChainImageViews[i]),"create Image view Failed");
    }

    //Create Semophere
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphores!");
    }

}

void Application::shutdownVulkan() {

    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    imageAvailableSemaphore=VK_NULL_HANDLE;
    renderFinishedSemaphore= VK_NULL_HANDLE;
    for(auto& imageView :swapChainImageViews){
        vkDestroyImageView(device,imageView, nullptr);
    }
    swapChainImageViews.clear();
    swapChainImages.clear();

    vkDestroySwapchainKHR(device,swapchain, nullptr);
    swapchain= VK_NULL_HANDLE;
    vkDestroyDevice(device, nullptr);

    device=VK_NULL_HANDLE;
    vkDestroyDebugMessenger(instance,debugMessenger, nullptr);

    debugMessenger = VK_NULL_HANDLE;
    vkDestroySurfaceKHR(instance,surface, nullptr);
    instance= VK_NULL_HANDLE ;

    vkDestroyInstance(instance, nullptr);
    surface = VK_NULL_HANDLE;

}

void Application::shutdownWindow() {
    glfwDestroyWindow(window);
    window = nullptr;
}

void Application::RenderFrame(){
    uint32_t imageIndex =0;
    vkAcquireNextImageKHR(device,swapchain,std::numeric_limits<uint64_t>::max(),
                          imageAvailableSemaphore,VK_NULL_HANDLE,&imageIndex);

    VkCommandBuffer commandBuffer = render->render(imageIndex);

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    VkSwapchainKHR swapChains[] = {swapchain};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional
    vkQueuePresentKHR(presentQueue, &presentInfo);
}

void Application::mainLoop() {
    if(!window)
        return;

    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();
        RenderFrame();
    }

    vkDeviceWaitIdle(device);
}


void Application::initVulkanExtensions() {
    vkDestroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,"vkDestroyDebugUtilsMessengerEXT");
    if(!vkDestroyDebugMessenger)
        throw std::runtime_error("Create DestroyDebugUtilsMessengerEXT function Failed\n");

    vkCreateDebugMessenger = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");
    if(!vkCreateDebugMessenger)
        throw std::runtime_error("Create CreateDebugUtilsMessengerEXT function Failed\n");

}

QueueFamilyIndices Application::fetchFamilyIndices(VkPhysicalDevice physical_device) {
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,&queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,&queueCount,queueFamilies.data());
    int i = 0;
    QueueFamilyIndices indices;
    for (const auto& queueFamily : queueFamilies) {
        VkBool32  presentSupport=false;
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = std::make_pair(true,i);
        }

        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device,i,surface,&presentSupport);
        if(queueFamily.queueCount > 0 && presentSupport){
            indices.presentFamily = std::make_pair(true,i);
        }
        if(indices.isComplete()){
            break;
        }
        i++;
    }
    return indices;
}


SwapchainSupportedDetails Application::fetchSwapchainSupportedDetails(VkPhysicalDevice& physical_device,
                                                                      VkSurfaceKHR& v_surface) {
    SwapchainSupportedDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device,surface,&details.capabilities);

    uint32_t  formatCount =0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,v_surface,&formatCount, nullptr);
    if(formatCount > 0){
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,v_surface,&formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,surface,&presentModeCount, nullptr);
    if(presentModeCount > 0){
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,surface,&presentModeCount, details.presentModes.data());
    }

    return details;
}

SwapchainSettings Application::selectOptimalSwapchainSettings(SwapchainSupportedDetails& details) {
    SwapchainSettings settings;

    //select best format if the surface has no preferred format
    if(details.formats.size() == 1 && details.formats[0].format == VK_FORMAT_UNDEFINED){
        settings.format = {VK_FORMAT_B8G8R8A8_UNORM,
                           VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    }else {
        //Select one of available formats
        size_t idx = 0;
        settings.format = details.formats[0];
        for (const auto &format: details.formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                settings.format = format;
                break;
            }
        }
    }

    settings.presentMode  = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& presentMode :details.presentModes) {

        if(presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR){
            settings.presentMode =presentMode;
        }
        if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR){
            settings.presentMode = presentMode;
            break;
        }
    }
    //select swap current extent
    if(details.capabilities.currentExtent.width !=std::numeric_limits<uint32_t>::max()){
        settings.extent = details.capabilities.currentExtent;
    }else{
        //Manually set extent match
        VkSurfaceCapabilitiesKHR& capabilities =details.capabilities;
        settings.extent = { Width,Height};
        settings.extent.width = std::clamp(settings.extent.width,
                                           details.capabilities.minImageExtent.width,
                                           details.capabilities.maxImageExtent.width);
        settings.extent.height = std::clamp(settings.extent.height,
                                           details.capabilities.minImageExtent.height,
                                           details.capabilities.maxImageExtent.height);

    }

    return settings;
}

void Application::shutdownRender() {
    render->shutdown();
}

void Application::initRender() {
    RenderContext context ;
    context.device_ = device;
    context.extend = swapChainExtent;
    context.format= swapChainImageFormat;
    context.imageViews= swapChainImageViews;
    context.queueFamilyIndex = fetchFamilyIndices(physicalDevice);
    render = new Render(context);
    render->init(R"(C:\Users\y123456\Desktop\Programming\c_cpp\GameEngine\shaders\vert.spv)",
                 R"(C:\Users\y123456\Desktop\Programming\c_cpp\GameEngine\shaders\frag.spv)");


}


int main(){
   if(!glfwInit())
       return EXIT_FAILURE;
   try {
       Application app;
       app.run();
   }catch (const std::exception& e){
       std::cerr<<e.what()<<std::endl;
       glfwTerminate();
       return EXIT_FAILURE;
   }
    glfwTerminate();
    return EXIT_SUCCESS;
}

