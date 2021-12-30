//
// Created by y123456 on 2021/10/15.
//

#include "Device.h"
#include "Macro.h"
#include "Utils.h"
#include <GLFW/glfw3.h>
#include <array>
#include <cstring>
#include <iostream>
#include <set>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace render::backend::vulkan {
std::vector<const char *> requiredPhysicalDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

std::vector<const char *> requiredValidationLayers = {
    "VK_LAYER_KHRONOS_validation"};

static int maxCombinedImageSamplers = 256;
static int maxUniformBuffers = 256;
static int maxDescriptorSet = 1024;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

namespace {

VkApplicationInfo createApplicationInfo(const char *app_name, const char *engine_name)
{
    //填充应用信息
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = app_name;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = engine_name;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    return appInfo;
}

VkDebugUtilsMessengerCreateInfoEXT createDebugMessengerCreateInfo(
    VkApplicationInfo &appInfo)
{
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {};
    debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMessengerInfo.pfnUserCallback = debugCallback;
    debugMessengerInfo.pUserData = nullptr;
    return debugMessengerInfo;
}

VkInstance createInstance(std::vector<const char *> &extensions,
                          std::vector<const char *> &layers,
                          VkApplicationInfo &appInfo,
                          VkDebugUtilsMessengerCreateInfoEXT &debugMessengerInfo)
{
    VkInstance instance;
    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = extensions.size();
    instanceInfo.ppEnabledExtensionNames = extensions.data();
    instanceInfo.enabledLayerCount = layers.size();
    instanceInfo.ppEnabledLayerNames = layers.data();
    instanceInfo.pNext = &debugMessengerInfo;
    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &instance), "Failed to Create Instance\n");
    return instance;
}


VkDevice createDevice(
    VkPhysicalDevice physicalDevice,
    VkDeviceQueueCreateInfo &queuesInfo,
    std::vector<const char *> &requiredPhysicalExtensions,
    std::vector<const char *> &layers)
{
    VkDevice device;
    VkDeviceCreateInfo deviceCreateInfo = {};
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_FALSE;
    deviceFeatures.sampleRateShading = VK_TRUE;
    deviceFeatures.tessellationShader = VK_TRUE;
    deviceFeatures.geometryShader = VK_TRUE;

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queuesInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = requiredPhysicalExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = requiredPhysicalExtensions.data();
    deviceCreateInfo.enabledLayerCount = layers.size();
    deviceCreateInfo.ppEnabledLayerNames = layers.data();
    VK_CHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device), "Create logical Device failed\n");
    return device;
}

VkSurfaceKHR createSurface(VkInstance &instance, GLFWwindow *window)
{
    VkSurfaceKHR surface;
    VK_CHECK(glfwCreateWindowSurface(instance, window, nullptr, &surface), "Create Surface Failed");
    return surface;
}

}// namespace

Device::Device()
{
}

Device::~Device()
{
    shutdown();
}

void Device::wait()
{
    vkDeviceWaitIdle(device);
}

void Device::init(const char *app_name, const char *engine_name)
{
    VK_CHECK(volkInitialize(), "can not init volk help lib");
    //Get Instance extension
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    //TODO use platform::createSurface()
    std::vector<const char *> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    TH_WITH_MSG(!Utils::checkInstanceExtensions(requiredExtensions, true),
                "This device is not have Vulkan Extension");

    //Validation Layer
    TH_WITH_MSG(!Utils::checkInstanceValidationLayers(requiredValidationLayers, true),
                "This device does not have vulkan Validation layer supported");

    auto appInfo = createApplicationInfo(app_name, engine_name);

    auto debugMessengerInfo = createDebugMessengerCreateInfo(appInfo);

    instance = createInstance(requiredExtensions, requiredValidationLayers, appInfo, debugMessengerInfo);

    volkLoadInstance(instance);

    // Create Debug Message
    VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &debugMessengerInfo, nullptr, &debugMessenger),
             " CreateDebugUtilsMessengerEXT  Failed\n");

    physicalDevice = PickPhysicalDevice(instance);
    graphicsQueueFamily = Utils::getGraphicsQueueFamily(physicalDevice);
    VkDeviceQueueCreateInfo graphicsQueueInfo = {};
    static const float queuePriority = 1.0f;
    graphicsQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphicsQueueInfo.queueFamilyIndex = graphicsQueueFamily;
    graphicsQueueInfo.queueCount = 1;
    graphicsQueueInfo.pQueuePriorities = &queuePriority;

    device = createDevice(physicalDevice, graphicsQueueInfo, requiredPhysicalDeviceExtensions, requiredValidationLayers);
    volkLoadDevice(device);

    //Get Logical Device
    vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
    TH_WITH_MSG(graphicsQueue == VK_NULL_HANDLE, "Get graphics queue from logical device failed");

    ////Create CommandPool
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = graphicsQueueFamily;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;// Optional
    VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool), "failed to create command pool!");

    //create descriptor Pool
    std::array<VkDescriptorPoolSize, 2> descriptorPoolSizes{};
    descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSizes[0].descriptorCount = maxUniformBuffers;
    descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSizes[1].descriptorCount = maxCombinedImageSamplers;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
    descriptorPoolCreateInfo.maxSets = maxDescriptorSet;

    descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VK_CHECK(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool),
             "failed to create descriptor pool!");
    maxMSAASamples = Utils::getMaxUsableSampleCount(physicalDevice);

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;

    if (vmaCreateAllocator(&allocatorInfo, &vram_allocator) != VK_SUCCESS)
        throw std::runtime_error("Can't create VRAM allocator");
}

int Device::checkPhysicalDevice(VkPhysicalDevice physical_device)
{
    if (!Utils::checkPhysicalDeviceExtensions(physical_device, requiredPhysicalDeviceExtensions)) {
        return -1;
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physical_device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physical_device, &deviceFeatures);

    int count = 0;

    switch (deviceProperties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
    case VK_PHYSICAL_DEVICE_TYPE_CPU: {
        count = 10;
    } break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
        count = 100;
    } break;
    default: {
        count = 1000;
    } break;
    }
    if (!deviceFeatures.geometryShader)
        count++;

    if (!deviceFeatures.tessellationShader)
        count++;

    return count;
}

VkPhysicalDevice Device::PickPhysicalDevice(VkInstance instance)
{
    //枚举物理设备
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    TH_WITH_MSG(deviceCount == 0, "No Support Vulkan Physical Device\n");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    //TODO Pick the best Physical Device
    VkPhysicalDevice best_device = VK_NULL_HANDLE;
    int GPU = -1;
    for (const auto &physical_device : physicalDevices) {
        int current_gpu = checkPhysicalDevice(physical_device);
        if (current_gpu == -1)
            continue;
        if (GPU > current_gpu)
            continue;

        GPU = current_gpu;
        best_device = physical_device;
    }
    return best_device;
}

void Device::shutdown()
{
    vmaDestroyAllocator(vram_allocator);
    vram_allocator = VK_NULL_HANDLE;
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    descriptorPool = VK_NULL_HANDLE;

    vkDestroyCommandPool(device, commandPool, nullptr);
    commandPool = VK_NULL_HANDLE;
    //    vkDestroyDevice(device, nullptr);
    //    device=VK_NULL_HANDLE;
    //    vkDestroyDebugUtilsMessengerEXT(instance,debugMessenger, nullptr);

    graphicsQueueFamily = 0xFFFF;
    graphicsQueue = VK_NULL_HANDLE;

    //    vkDestroyInstance(instance, nullptr);
    //    instance= VK_NULL_HANDLE;
}
}// namespace render::backend::vulkan