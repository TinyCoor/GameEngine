//
// Created by y123456 on 2021/10/11.
//
#include "VulkanApplication.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderScene.h"
#include "VulkanRender.h"
#include "VulkanImGuiRender.h"
#include "Macro.h"
#include "VulkanUtils.h"
#include <iostream>
#include <set>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <algorithm>
#include <functional>
#include <volk.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>


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
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.hwnd = glfwGetWin32Window(window);
        surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
        VK_CHECK(vkCreateWin32SurfaceKHR(instance,&surfaceCreateInfo, nullptr,&surface),"Create win32 Surface Error");
        return surface;
    }
}



void Application::initWindow() {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GL_RED_BITS,mode->redBits);
    glfwWindowHint(GL_GREEN_BITS,mode->greenBits);
    glfwWindowHint(GL_BLUE_BITS,mode->blueBits);
    glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);

    window = glfwCreateWindow(mode->width,mode->height,"Vulkan", nullptr, nullptr);

    glfwSetWindowUserPointer(window,this);
    glfwSetFramebufferSizeCallback(this->window,&Application::OnFrameBufferResized);
}

void Application::run(){
    initWindow();
    initImGui();
    initVulkan();
    initVulkanSwapChain();
    initScene();
    initRender();
    initImGuiRender();
    mainLoop();
    shutdownImGuiRender();
    shutdownWindow();
    shutdownScene();
    shutdownRender();
    shutdownSwapChain();
    shutdownImGui();
    shutdownVulkan();
}

Application::~Application(){
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

    QueueFamilyIndices indices = VulkanUtils::fetchFamilyIndices(physical_device,context.surface);
    if(!indices.isComplete())
        return false;

    std::vector<const char*> deviceExtensions;
    if( !checkRequiredPhysicalDeviceExtensions(physical_device,deviceExtensions)){
        return false;
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

    if(!deviceFeatures.geometryShader)
        return false;

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

void Application::initVulkan() {

    // VK_CHECK( volkInitialize(),"can not init volk help lib");
    //检查vulkan extension and layer
    std::vector<const char*> extensions;
    TH_WITH_MSG(!checkRequiredExtension(extensions),"This device is not have Vulkan Extension\n");

    std::vector<const char* > layers;
    TH_WITH_MSG(!checkValidationLayers(layers),"This device does not have vulkan Validation layer supported\n");

    auto appInfo =createApplicationInfo();
    auto debugMessengerInfo = createDebugMessengerCreateInfo(appInfo);

    context.instance = createInstance(extensions,layers,appInfo,debugMessengerInfo);
    volkLoadInstance(context.instance);

    VK_CHECK(vkCreateDebugUtilsMessengerEXT(context.instance, &debugMessengerInfo, nullptr, &debugMessenger)," CreateDebugUtilsMessengerEXT  Failed\n");
    //create Vulkan Surface TODO cross platform support
    context.surface = createSurface(context.instance,window);
    //枚举物理设备
    uint32_t deviceCount =0;
    vkEnumeratePhysicalDevices(context.instance,&deviceCount, nullptr);
    TH_WITH_MSG(deviceCount == 0,"No Support Vulkan Physical Device\n");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(context.instance,&deviceCount, physicalDevices.data());

    //TODO Pick the best Physical Device
    for (const auto& physical_device :physicalDevices) {
        if(checkPhysicalDevice(physical_device,context.surface)){
            context.physicalDevice= physical_device;
            break;
        }
    }

    TH_WITH_MSG( context.physicalDevice == VK_NULL_HANDLE,"failed to find GPU\n");

    QueueFamilyIndices indices = VulkanUtils::fetchFamilyIndices(context.physicalDevice,context.surface);
    auto queuesInfo = createDeviceQueueCreateInfo(indices);

    context.device_= createDevice(context.physicalDevice ,queuesInfo,requiredPhysicalDeviceExtensions,layers);
    volkLoadDevice(context.device_);

    //Get Logical Device
    vkGetDeviceQueue(context.device_,indices.graphicsFamily.value(),0,&graphicsQueue);
    TH_WITH_MSG(graphicsQueue == VK_NULL_HANDLE,"Get graphics queue from logical device failed\n");

    vkGetDeviceQueue(context.device_,indices.presentFamily.value(),0,&presentQueue);
    TH_WITH_MSG(presentQueue == VK_NULL_HANDLE,"Get present queue from logical device failed\n");


    ////
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = indices.graphicsFamily.value();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional
    VK_CHECK(vkCreateCommandPool(context.device_, &commandPoolInfo, nullptr, &commandPool),"failed to create command pool!");


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

    VK_CHECK(vkCreateDescriptorPool(context.device_, &descriptorPoolCreateInfo, nullptr, &descriptorPool),"failed to create descriptor pool!");


    context.commandPool = commandPool;
    context.graphicsQueueFamily= indices.graphicsFamily.value();
    context.presentQueueFamily = indices.presentFamily.value();
    context.graphicsQueue = graphicsQueue;
    context.presentQueue= presentQueue;
    context.maxMSAASamples = VulkanUtils::getMaxUsableSampleCount(context);
    context.descriptorPool =descriptorPool;
}

void Application::shutdownVulkan() {

    vkDestroyDescriptorPool(context.device_, descriptorPool, nullptr);
    descriptorPool = VK_NULL_HANDLE;

    vkDestroyCommandPool(context.device_,commandPool, nullptr);
    commandPool = VK_NULL_HANDLE;

    vkDestroyDebugUtilsMessengerEXT(context.instance,debugMessenger, nullptr);

    vkDestroyDevice(context.device_, nullptr);
    context.device_=VK_NULL_HANDLE;

    vkDestroySurfaceKHR(context.instance,context.surface, nullptr);
    context.instance= VK_NULL_HANDLE ;

    vkDestroyInstance(context.instance, nullptr);
    context.surface = VK_NULL_HANDLE;

}

void Application::initImGui() {
//    IMGUI_CHECKVERSION();
//    ImGui::CreateContext();
//    ImGuiIO& io = ImGui::GetIO();
//    ImGui::StyleColorsDark();
//
//    ImGui_ImplGlfw_InitForVulkan(window,true);

}

void Application::initImGuiRender() {
    if (!ImGuiRender){
        ImGuiRender = new VulkanImGuiRender(context);
       // ImGuiRender->init(state,scene,swapChain.get());
    }
}

void Application::shutdownImGuiRender() {
    if(ImGuiRender){
      //  ImGuiRender->shutdown();
        delete ImGuiRender;
        ImGuiRender = nullptr;
    }
}


void Application::shutdownImGui() {

 //   ImGui::DestroyContext();

}



void Application::shutdownWindow() {
    glfwDestroyWindow(window);
    window = nullptr;
}

void Application::RenderFrame(){
    VulkanRenderFrame frame;
    if(!swapChain->Acquire(state,frame)){
        recreateSwapChain();
        return;
    }


    ImGuiRender->render(state,scene,frame);
    render->render(state,scene, frame);

    if(!swapChain->Present(frame) || windowResized){
        windowResized = false;
        recreateSwapChain();
    }

}
void Application::update()
{
    render->update(state,scene);
   // ImGuiRender->update(state,scene);
}
void Application::mainLoop() {
    if(!window)
        return;

    while (!glfwWindowShouldClose(window)){
//         ImGui_ImplGlfw_NewFrame();
//         ImGui::NewFrame();

         update();

//        ImGui::Render();

        RenderFrame();
        glfwPollEvents();
    }

    vkDeviceWaitIdle(context.device_);
}


void Application::shutdownRender() {
    render->shutdown();
    delete render;
    render = nullptr;
}

void Application::initRender() {
    render = new VulkanRender(context,swapChain->getExtent(),swapChain->getDescriptorSetLayout(),swapChain->getRenderPass());
    render->init(state,scene);
}


void Application::initVulkanSwapChain() {
  //TODO
    if (!swapChain){
        swapChain= std::shared_ptr<VulkanSwapChain>(new VulkanSwapChain(context));
    }

    int width,height;
    glfwGetWindowSize(window,&width,&height);
    swapChain->init(sizeof(RenderState),width,height);

}

void Application::shutdownSwapChain() {
   //TODO
   if(swapChain){
       swapChain->shutdown();
   }
}

void Application::initScene() {
    scene = new VulkanRenderScene(context);
    scene->init();
}

void Application::shutdownScene() {
    scene->shutdown();
    delete scene;
    scene = nullptr;
}

void Application::recreateSwapChain() {
    int width =0;
    int height = 0;
    while (width ==0 || height == 0){
        glfwGetFramebufferSize(window,&width,&height);
        glfwPollEvents();
    }
    vkDeviceWaitIdle(context.device_);

    shutdownSwapChain();
    initVulkanSwapChain();
}


void Application::OnFrameBufferResized(GLFWwindow *window, int width, int height) {
    Application* app =(Application*)glfwGetWindowUserPointer(window);
    assert(app != nullptr);
    app->windowResized= true;
}
