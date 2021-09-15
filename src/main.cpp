#include "macro.h"

#include <iostream>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <cassert>


VkPhysicalDevice pickPhysicalDevice(VkPhysicalDevice device);

VkPhysicalDevice pickPhysicalDevice(VkPhysicalDevice* physicalDevices, uint32_t count);

VkInstance createInstance(){
    VkApplicationInfo appinfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appinfo.apiVersion=VK_API_VERSION_1_1;

    VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo=&appinfo;
#ifdef NDEBUG
    const char* debugLayers[]={
            "VK_LAYER_LUNARG_standard_validation"
    };
    createInfo.pApplicationInfo= &appinfo;
    createInfo.ppEnabledLayerNames = debugLayers;
    createInfo.enabledLayerCount =sizeof(debug_layers)/sizeof (debug_layers[0]);
#endif
//    uint32_t glfwExtensionCount = 0;
//    const char** glfwExtensions;
//    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
//
//    createInfo.enabledExtensionCount = glfwExtensionCount;
//    createInfo.ppEnabledExtensionNames = glfwExtensions;


    const char* extensions[]={
        VK_KHR_SURFACE_EXTENSION_NAME
    };
    createInfo.ppEnabledExtensionNames= extensions;
    createInfo.enabledExtensionCount= sizeof(extensions)/sizeof(extensions[0]);

    VkInstance  instance= 0;
    VK_CHECK(vkCreateInstance(&createInfo, nullptr,&instance));
    return instance;

}

VkSurfaceKHR createSurface(VkInstance instance,GLFWwindow* window){
#if defined( VK_USE_PLATFORM_WIN32_KHR)
    VkWin32SurfaceCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    VkSurfaceKHR  surface =0;
    createInfo.hwnd= glfwGetWin32Window(window);
    createInfo.hinstance= GetModuleHandle(nullptr);
    VK_CHECK(vkCreateWin32SurfaceKHR(instance,&createInfo, nullptr,&surface));

#else
#error "Unsupport Platform"
#endif

    return nullptr;

}

int main() {
    int rc = glfwInit();
    (void)rc;
    VkInstance instance = createInstance();
    VkPhysicalDevice physicalDevices[16];
    uint32_t physicalDeviceCount=sizeof (physicalDevices)/sizeof(physicalDevices[0]);
    VK_CHECK( vkEnumeratePhysicalDevices(instance,&physicalDeviceCount,physicalDevices));

    VkPhysicalDevice physicalDevice = pickPhysicalDevice(physicalDevices,physicalDeviceCount);
    assert(physicalDevice);

    float queueuPriority[] ={1.0};


   // vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,);
    VkDeviceQueueCreateInfo queueCreateInfo {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueCreateInfo.queueFamilyIndex = 0;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queueuPriority;


    VkDevice device;
    VkDeviceCreateInfo deviceCreateInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.pQueueCreateInfos= &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    VK_CHECK(vkCreateDevice(physicalDevice,&deviceCreateInfo, nullptr,&device));


    GLFWwindow* window =glfwCreateWindow(1024,768,"niagara",0,0);
    assert(window != nullptr);

    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();
    }

    return 0;
}

VkPhysicalDevice pickPhysicalDevice(VkPhysicalDevice* physicalDevices, uint32_t count) {
    for (uint32_t idx =0; idx<count; ++idx){
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices[idx],&properties);
        if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            printf("Picking discrate Gpu %s\n",properties.deviceName);
            return physicalDevices[idx];
        }
    }

    if(count > 0){
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices[0],&properties);
        printf("Picking fallback Gpu %s\n",properties.deviceName);
        return physicalDevices[0];
    }
    printf("No Physical Device Available\n");
    return VK_NULL_HANDLE;
}

