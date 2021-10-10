//
// Created by y123456 on 2021/10/10.
//
#include <stdexcept>
#include <iostream>
#include <optional>
#include <vector>
#include <vulkan.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/vec4.hpp>

#define VK_CHECK(call,msg)                              \
    do {                                                \
        VkResult res  =call;                            \
        if(res == VK_SUCCESS ){                          \
             throw std::runtime_error(msg);               \
        }                                                 \
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

struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;

    bool isComplete()const{
        return graphicsFamily.has_value();
    }


};

class Application{
public:
    void run();

private:

    bool checkValidationLayers(std::vector<const char*>& layers)const;
    bool checkRequiredExtension(std::vector<const char*>& extensions)const;
    bool checkPhysicalDevice(VkPhysicalDevice device) const;
    QueueFamilyIndices fetchFamilyIndices(VkPhysicalDevice physical_device) const;
    void initWindow();
    void initVulkan();
    void initVulkanExtensions();
    void shutdownVulkan();
    void mainLoop();
    void shutdownWindow();

private:
    GLFWwindow* window{nullptr};


    VkInstance instance{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphicsQueue {VK_NULL_HANDLE};
    VkSurfaceKHR surface {0};
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};


    static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugMessenger;
    static PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugMessenger;
};

PFN_vkCreateDebugUtilsMessengerEXT Application::vkCreateDebugMessenger{VK_NULL_HANDLE};
PFN_vkDestroyDebugUtilsMessengerEXT Application::vkDestroyDebugMessenger{VK_NULL_HANDLE};

void Application::initWindow() {
    glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
    const int width =800;
    const int height = 600;
    window = glfwCreateWindow(width,height,"Vulkan", nullptr, nullptr);
}

void Application::run(){
    initWindow();
    initVulkan();
    mainLoop();
    shutdownVulkan();
    shutdownWindow();
}

bool Application::checkRequiredExtension(std::vector<const char*>& extensions)const {

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

bool Application::checkValidationLayers(std::vector<const char *> &layers) const {
    uint32_t vulkanLayerCount =0;
    vkEnumerateInstanceLayerProperties(&vulkanLayerCount, nullptr);
    std::vector<VkLayerProperties> vulkanLayers(vulkanLayerCount);
    vkEnumerateInstanceLayerProperties(&vulkanLayerCount, vulkanLayers.data());

    static std::vector<const char*> requiredLayers={
            "VK_LAYER_KHRONOS_validation"
    };
    layers.clear();
    for (const auto& requiredLayer :requiredLayers) {
        bool supported = false;
        for(const auto& layer : vulkanLayers){
            if(strcmp(requiredLayer,layer.layerName) ==0){
                supported = true;
                break;
            }
        }
        if(!supported)
            return false;
        layers.push_back(requiredLayer);
    }
    return true;
}


void Application::initVulkan() {
    //检查vulkan extension and layer
    std::vector<const char*> extensions;
    if(!checkRequiredExtension(extensions))
        throw std::runtime_error("This device is not have Vulkan Extension\n");

    std::vector<const char* > layers;
    if(!checkValidationLayers(layers))
        throw std::runtime_error("This device does not have vulkan Validation layer supported\n");


    //填充应用信息
    VkApplicationInfo appInfo={};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName ="PBR Render";
    appInfo.applicationVersion= VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    //
    VkDebugUtilsMessengerCreateInfoEXT  debugMessengerInfo={};
    debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerInfo.messageSeverity =VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                     | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                     | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMessengerInfo.pfnUserCallback = debugCallback;
    debugMessengerInfo.pUserData = nullptr;

    VkInstanceCreateInfo instanceInfo={};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = extensions.size();
    instanceInfo.ppEnabledExtensionNames =extensions.data();
    instanceInfo.enabledLayerCount = layers.size();
    instanceInfo.ppEnabledLayerNames = layers.data();
    instanceInfo.pNext = &debugMessengerInfo;
    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr,&instance),"Failed to Create Instance\n");
    initVulkanExtensions();

    //create Vulkan Surface
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hwnd = glfwGetWin32Window(window);
    surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

    VK_CHECK(vkCreateWin32SurfaceKHR(instance,&surfaceCreateInfo, nullptr,&surface),"Create Surface Error");
    VK_CHECK(vkCreateDebugMessenger(instance, &debugMessengerInfo, nullptr, &debugMessenger)," CreateDebugUtilsMessengerEXT  Failed\n");


    //枚举物理设备
    uint32_t deviceCount =0;
    vkEnumeratePhysicalDevices(instance,&deviceCount, nullptr);
    if(deviceCount == 0){
        throw std::runtime_error("No Support Vulkan Physical Device\n");
    }
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance,&deviceCount, physicalDevices.data());

    for (const auto& physical_device :physicalDevices) {
        if(checkPhysicalDevice(physical_device)){
            physicalDevice= physical_device;
            break;
        }
    }
    if(physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("failed to find GPU\n");

    QueueFamilyIndices indices = fetchFamilyIndices(physicalDevice);
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures ={};
   // vkGetPhysicalDeviceFeatures(physical_device,&deviceFeatures);

    VkDeviceCreateInfo deviceCreateInfo={};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = 0;
    deviceCreateInfo.enabledLayerCount = layers.size();
    deviceCreateInfo.ppEnabledLayerNames =layers.data();

    VK_CHECK(vkCreateDevice(physicalDevice,&deviceCreateInfo, nullptr,&device),"Create logical Device failed\n");

    vkGetDeviceQueue(device,indices.graphicsFamily.value(),0,&graphicsQueue);

}

void Application::shutdownVulkan() {
    vkDestroyDevice(device, nullptr);
    device=VK_NULL_HANDLE;
    vkDestroyDebugMessenger(instance,debugMessenger, nullptr);
    debugMessenger = VK_NULL_HANDLE;
    vkDestroyInstance(instance, nullptr);
    instance= VK_NULL_HANDLE ;
}

void Application::mainLoop() {
    if(!window)
        return;

    while (!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1.0,0.0,0.0,1.0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Application::shutdownWindow() {
    glfwDestroyWindow(window);
    window = nullptr;
}

void Application::initVulkanExtensions() {

    vkDestroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,"vkDestroyDebugUtilsMessengerEXT");
    if(!vkDestroyDebugMessenger)
        throw std::runtime_error("Create DestroyDebugUtilsMessengerEXT function Failed\n");

    vkCreateDebugMessenger = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");
    if(!vkCreateDebugMessenger)
        throw std::runtime_error("Create CreateDebugUtilsMessengerEXT function Failed\n");

}

//TODO fix
bool Application::checkPhysicalDevice(VkPhysicalDevice physical_device) const {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physical_device,&deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physical_device,&deviceFeatures);

    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,&queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,&queueCount,queueFamilies.data());

    QueueFamilyIndices indices = fetchFamilyIndices(physical_device);
    if(!indices.isComplete())
        return false;

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
    && deviceFeatures.geometryShader;
}

QueueFamilyIndices Application::fetchFamilyIndices(VkPhysicalDevice physical_device) const {
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,&queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,&queueCount,queueFamilies.data());
    int i = 0;
    QueueFamilyIndices indices;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        i++;
    }
    return indices;
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

