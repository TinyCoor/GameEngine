//
// Created by y123456 on 2021/10/15.
//

#include "VulkanContext.h"
#include "VulkanUtils.h"
#include "Macro.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include <cstring>
#include <set>
#include <array>
#include <GLFW/glfw3native.h>

std::vector<const char*>  requiredPhysicalDeviceExtensions ={
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

std::vector<const char*> requiredValidationLayers ={
        "VK_LAYER_KHRONOS_validation"
};

static int maxCombinedImageSamplers = 32;
static int maxUniformBuffers = 32;


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}


namespace {

    VkApplicationInfo createApplicationInfo(){
        //填充应用信息
        VkApplicationInfo appInfo={};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName ="PBR VulkanRender";
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
        debugMessengerInfo.messageSeverity =  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugMessengerInfo.messageType =VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
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
        std::set<uint32_t > uniqueQueueFamilies={ indices.graphicsFamily.value(),indices.presentFamily.value()};

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
        deviceFeatures.samplerAnisotropy = VK_FALSE;
        deviceFeatures.sampleRateShading = VK_TRUE;

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
        VK_CHECK(glfwCreateWindowSurface(instance,window, nullptr,&surface),"Create Surface Failed");
//        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
//        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
//        surfaceCreateInfo.hwnd = glfwGetWin32Window(window);
//        surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
//        VK_CHECK(vkCreateWin32SurfaceKHR(instance,&surfaceCreateInfo, nullptr,&surface),"Create win32 Surface Error");
//
         return surface;
    }

}


void VulkanContext::init(GLFWwindow* window) {

    VK_CHECK(volkInitialize(),"can not init volk help lib");

    //Get Instance extension
    uint32_t glfwExtensionCount =0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> requiredExtensions(glfwExtensions,glfwExtensions + glfwExtensionCount);
    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    TH_WITH_MSG(!VulkanUtils::checkInstanceExtensions(requiredExtensions, true),
                "This device is not have Vulkan Extension");

    //Validation Layer
    TH_WITH_MSG(!VulkanUtils::checkInstanceValidationLayers(requiredValidationLayers,true),
                "This device does not have vulkan Validation layer supported");

    auto appInfo =createApplicationInfo();

    auto debugMessengerInfo = createDebugMessengerCreateInfo(appInfo);

    instance = createInstance(requiredExtensions,requiredValidationLayers,appInfo,debugMessengerInfo);

    volkLoadInstance(instance);

//    Create Debug Message
    VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &debugMessengerInfo, nullptr, &debugMessenger),
             " CreateDebugUtilsMessengerEXT  Failed\n");

    //create Vulkan Surface
    // TODO cross platform support
    surface = createSurface(instance,window);

    physicalDevice = PickPhysicalDevice(instance,surface);
    TH_WITH_MSG( physicalDevice == VK_NULL_HANDLE,"failed to find GPU\n");

    //获取家族队列信息及支持task 也支持graphics
    QueueFamilyIndices indices = VulkanUtils::fetchFamilyIndices(physicalDevice,surface);
    auto queuesInfo = createDeviceQueueCreateInfo(indices);


    device= createDevice(physicalDevice ,queuesInfo,requiredPhysicalDeviceExtensions,requiredValidationLayers);
    volkLoadDevice(device);

    //Get Logical Device
    vkGetDeviceQueue(device,indices.graphicsFamily.value(),0,&graphicsQueue);
    TH_WITH_MSG(graphicsQueue == VK_NULL_HANDLE,"Get graphics queue from logical device failed");

    vkGetDeviceQueue(device,indices.presentFamily.value(),0,&presentQueue);
    TH_WITH_MSG(presentQueue == VK_NULL_HANDLE,"Get present queue from logical device failed");


    ////Create CommandPool
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = indices.graphicsFamily.value();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional
    VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool),"failed to create command pool!");


    //create descriptor Pool
    std::array<VkDescriptorPoolSize,2> descriptorPoolSizes{};
    descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSizes[0].descriptorCount = maxUniformBuffers;
    descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSizes[1].descriptorCount = maxCombinedImageSamplers;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
    descriptorPoolCreateInfo.maxSets = maxCombinedImageSamplers + maxUniformBuffers;;
    descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VK_CHECK(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool),"failed to create descriptor pool!");
    maxMSAASamples= VulkanUtils::getMaxUsableSampleCount(physicalDevice);

}


//TODO Pick Suitable GPU
int VulkanContext::checkPhysicalDevice(VkPhysicalDevice physical_device,VkSurfaceKHR& v_surface) {

    QueueFamilyIndices indices = VulkanUtils::fetchFamilyIndices(physical_device,surface);
    if(!indices.isComplete())
        return -1;


    if( !VulkanUtils::checkPhysicalDeviceExtensions(physical_device,requiredPhysicalDeviceExtensions)){
        return -1;
    }

    uint32_t formatCount=0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,surface,&formatCount, nullptr);

    uint32_t present_count =0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,surface,&present_count, nullptr);

    if(formatCount ==0 || present_count == 0){
        return -1;
    }

    /*
     * TODO
    SwapchainSupportedDetails details = fetchSwapchainSupportedDetails(physical_device,surface);

    if(details.formats.empty() || details.presentModes.empty()){
        return false;
    }
     */

    //TODO only checks for test remove later
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physical_device,&deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physical_device,&deviceFeatures);


    int count = 0;

    switch (deviceProperties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        case VK_PHYSICAL_DEVICE_TYPE_CPU:{count =10;}break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:{count =100;}break;
        default: {
            count =1000;
        }break;

    }
    if(!deviceFeatures.geometryShader)
        count++;

    if (!deviceFeatures.tessellationShader)
        count++;

    return count;
}



VkPhysicalDevice VulkanContext::PickPhysicalDevice(VkInstance instance,VkSurfaceKHR surface){
    //枚举物理设备
    uint32_t deviceCount =0;
    vkEnumeratePhysicalDevices(instance,&deviceCount, nullptr);
    TH_WITH_MSG(deviceCount == 0,"No Support Vulkan Physical Device\n");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance,&deviceCount, physicalDevices.data());

    //TODO Pick the best Physical Device
    VkPhysicalDevice best_device = VK_NULL_HANDLE;
    int GPU = -1;
    for (const auto& physical_device :physicalDevices) {
        int current_gpu =  checkPhysicalDevice(physical_device,surface);
        if(current_gpu == -1)
            continue;
        if(GPU > current_gpu)
            continue;

        GPU = current_gpu;
        best_device = physical_device;
    }
    return best_device;
}

void VulkanContext::shutdown() {
    vkDestroyDescriptorPool(device,descriptorPool, nullptr);
    descriptorPool = VK_NULL_HANDLE;

    vkDestroyCommandPool(device,commandPool, nullptr);
    commandPool = VK_NULL_HANDLE;

//    vkDestroyDebugUtilsMessengerEXT(instance,debugMessenger, nullptr);
//

//    vkDestroyDevice(device, nullptr);
//    device=VK_NULL_HANDLE;
//
//    vkDestroySurfaceKHR(instance,surface, nullptr);
//    surface = VK_NULL_HANDLE;
//
//
//    vkDestroyInstance(instance, nullptr);
//    instance= VK_NULL_HANDLE;
}

VulkanContext::VulkanContext() {

}

VulkanContext::~VulkanContext() {
    shutdown();
}
