//
// Created by y123456 on 2021/10/11.
//

#include "VulkanApplication.h"
#include<iostream>
#include <set>
#include <GLFW/glfw3.h>

#include <GLFW/glfw3native.h>
#include <algorithm>
#include <functional>

const std::string vertex_shader_path = R"(C:\Users\y123456\Desktop\Programming\c_cpp\GameEngine\Resources\shaders\vert.spv)";
const std::string fragment_shader_path= R"(C:\Users\y123456\Desktop\Programming\c_cpp\GameEngine\Resources\shaders\frag.spv)";
const std::string texture_path = R"(C:\Users\y123456\Desktop\Programming\c_cpp\GameEngine\Resources\textures\viking_room.png)";
const std::string model_path= R"(C:\Users\y123456\Desktop\Programming\c_cpp\GameEngine\Resources\models\viking_room.obj)";


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
        deviceFeatures.samplerAnisotropy = VK_TRUE;
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

std::vector<const char*> Application:: requiredPhysicalDeviceExtensions ={
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

std::vector<const char*> Application::requiredValidationLayers={
        "VK_LAYER_KHRONOS_validation"
};

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
    initVulkan();
    initVulkanSwapChain();
    initScene();
    initRender();

    mainLoop();
    shutdownWindow();
    shutdownScene();
    shutdownSwapChain();
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

    if(!deviceFeatures.geometryShader)
        return false;

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}
void Application::initVulkan() {

    VK_CHECK( volkInitialize(),"can not init volk help lib");
    //检查vulkan extension and layer
    std::vector<const char*> extensions;
    TH_WITH_MSG(!checkRequiredExtension(extensions),"This device is not have Vulkan Extension\n");

    std::vector<const char* > layers;
    TH_WITH_MSG(!checkValidationLayers(layers),"This device does not have vulkan Validation layer supported\n");

    auto appInfo =createApplicationInfo();
    auto debugMessengerInfo = createDebugMessengerCreateInfo(appInfo);

    instance = createInstance(extensions,layers,appInfo,debugMessengerInfo);
    volkLoadInstance(instance);

    VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &debugMessengerInfo, nullptr, &debugMessenger)," CreateDebugUtilsMessengerEXT  Failed\n");
    //create Vulkan Surface TODO cross platform support
    surface = createSurface(instance,window);
    //枚举物理设备
    uint32_t deviceCount =0;
    vkEnumeratePhysicalDevices(instance,&deviceCount, nullptr);
    TH_WITH_MSG(deviceCount == 0,"No Support Vulkan Physical Device\n");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance,&deviceCount, physicalDevices.data());

    //TODO Pick the best Physical Device
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
    volkLoadDevice(device);

    //Get Logical Device
    vkGetDeviceQueue(device,indices.graphicsFamily.second,0,&graphicsQueue);
    TH_WITH_MSG(graphicsQueue == VK_NULL_HANDLE,"Get graphics queue from logical device failed\n");

    vkGetDeviceQueue(device,indices.presentFamily.second,0,&presentQueue);
    TH_WITH_MSG(presentQueue == VK_NULL_HANDLE,"Get present queue from logical device failed\n");


    //
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = indices.graphicsFamily.second;
    commandPoolInfo.flags = 0; // Optional
    VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool),"failed to create command pool!");

    //Create Sync Object
    VkSemaphoreCreateInfo semaphoreInfo{};
    imageAvailableSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAME_IN_FLIGHT);

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (size_t i = 0; i <MAX_FRAME_IN_FLIGHT ; ++i) {
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }

    context.device_ = device;
    context.physicalDevice = physicalDevice;
    context.commandPool = commandPool;
    context.graphicsQueue = graphicsQueue;
    context.presentQueue= presentQueue;
}

void Application::shutdownVulkan() {
    vkDestroyCommandPool(device,commandPool, nullptr);
    commandPool = VK_NULL_HANDLE;
    vkDestroyDebugUtilsMessengerEXT(instance,debugMessenger, nullptr);

    for (int i = 0; i <MAX_FRAME_IN_FLIGHT ; ++i) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
    imageAvailableSemaphores.clear();
    renderFinishedSemaphores.clear();
    inFlightFences.clear();
    vkDestroyDevice(device, nullptr);
    device=VK_NULL_HANDLE;

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
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    uint32_t imageIndex =0;

   VkResult result = vkAcquireNextImageKHR(
           device,
           swapchain,
           std::numeric_limits<uint64_t>::max(),
           imageAvailableSemaphores[currentFrame],VK_NULL_HANDLE,&imageIndex);

   if(result == VK_ERROR_OUT_OF_DATE_KHR){
        recreateSwapChain();
        return;
   }else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR ){
       throw std::runtime_error("can not recreate swap chain");
   }

    VkCommandBuffer commandBuffer = render->render(imageIndex);

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    vkResetFences(device,1,&inFlightFences[currentFrame]);
    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]),"failed to submit draw command buffer!");
    VkPresentInfoKHR presentInfo{};
    VkSwapchainKHR swapChains[] = {swapchain};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional
    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized){
        frameBufferResized  =true;
        recreateSwapChain();
    }else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR ){
        throw std::runtime_error("can not recreate swap chain");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
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

QueueFamilyIndices Application::fetchFamilyIndices(VkPhysicalDevice physical_device) {
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,&queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,&queueCount,queueFamilies.data());
    QueueFamilyIndices indices{};
    for (int i = 0; i< queueFamilies.size();++i) {

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = std::make_pair(true,i);
        }

        VkBool32  presentSupport= false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device,i,surface,&presentSupport);
        if(queueFamilies[i].queueCount > 0 && presentSupport){
            indices.presentFamily = std::make_pair(true,i);
        }
        if(indices.isComplete()){
            break;
        }
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
    SwapchainSettings settings{};

    //select best format if the surface has no preferred format
    //TODO RGBA
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
        int width =0 ,height =0;
        glfwGetFramebufferSize(window,&width,&height);
        settings.extent = { static_cast<uint32_t>(width),static_cast<uint32_t>(height)};
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
    VulkanSwapChainContext swapChainContext{};
    swapChainContext.depthImageView = depthImageView;
    swapChainContext.extend = swapChainExtent;
    swapChainContext.depthFormat= depthFormat;
    swapChainContext.colorFormat = swapChainImageFormat;
    swapChainContext.imageViews = swapChainImageViews;
    swapChainContext.descriptorPool =descriptorPool;

    render = new VulkanRender(context,swapChainContext);
    render->init(scene);

}


VkFormat Application::selectOptimalSupportedFormat(const std::vector<VkFormat>& candiates,
                                                   VkImageTiling tiling,
                                                   VkFormatFeatureFlags features)
{
    for(VkFormat format : candiates)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice,format,&properties);
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
            return format;

        if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
        std::cout << format<<"\n";
    }

    TH_WITH_MSG(true,"can not find support format");
}

VkFormat  Application::selectOptimalDepthFormat(){
    return selectOptimalSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void Application::initVulkanSwapChain() {
    //create SwapChain finally
    QueueFamilyIndices indices = fetchFamilyIndices(physicalDevice);
    SwapchainSupportedDetails details = fetchSwapchainSupportedDetails(physicalDevice,surface);
    SwapchainSettings settings = selectOptimalSwapchainSettings(details);

    uint32_t imageCount = details.capabilities.minImageCount + 1;

    if(details.capabilities.maxImageCount > 0 ){
        imageCount = std::min(imageCount,details.capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR swapChainInfo{};
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
        swapChainImageViews[i] = vulkanUtils::createImage2DView(context,
                                                                swapChainImages[i],
                                                                1,
                                                                swapChainImageFormat,
                                                                VK_IMAGE_ASPECT_COLOR_BIT);
    }


    //Create Depth Buffer
    depthFormat = selectOptimalDepthFormat();
    vulkanUtils::createImage2D(context,
                               swapChainExtent.width,
                               swapChainExtent.height,
                               1,
                               depthFormat,
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               depthImage, depthImageMemory
    );

    depthImageView = vulkanUtils::createImage2DView(context,
                                                    depthImage,
                                                    1,
                                                    depthFormat,
                                                    VK_IMAGE_ASPECT_DEPTH_BIT);

    vulkanUtils::transitionImageLayout(context,
                                       depthImage,
                                       1,
                                       depthFormat,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    //create descriptor Pool
    std::array<VkDescriptorPoolSize,2> descriptorPoolSizes{};
    descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSizes[0].descriptorCount = swapChainImageCount;
    descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSizes[1].descriptorCount = swapChainImageCount;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
    descriptorPoolCreateInfo.maxSets =swapChainImageCount;
    descriptorPoolCreateInfo.flags = 0;
    VK_CHECK(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool),"failed to create descriptor pool!");
}


void Application::shutdownSwapChain() {
    vkDestroyDescriptorPool(device,descriptorPool, nullptr);
    descriptorPool = VK_NULL_HANDLE;

    for(auto& imageView :swapChainImageViews){
        vkDestroyImageView(device,imageView, nullptr);
    }

    swapChainImageViews.clear();
    swapChainImages.clear();

    vkDestroyImageView(device,depthImageView, nullptr);
    depthImage = VK_NULL_HANDLE;

    vkDestroyImage(device,depthImage, nullptr);
    depthImageView =  VK_NULL_HANDLE;

    vkFreeMemory(device,depthImageMemory, nullptr);
    depthImageMemory = VK_NULL_HANDLE;

    vkDestroySwapchainKHR(device,swapchain, nullptr);
    swapchain= VK_NULL_HANDLE;
}

void Application::initScene() {
    scene = new VulkanRenderScene(context);
    scene->init(vertex_shader_path,fragment_shader_path,texture_path,model_path);
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
    vkDeviceWaitIdle(device);

    shutdownSwapChain();
    shutdownRender();

    initVulkanSwapChain();
    initRender();

}


void Application::OnFrameBufferResized(GLFWwindow *window, int width, int height) {
    Application* app =(Application*)glfwGetWindowUserPointer(window);
    assert(app != nullptr);
    app->frameBufferResized= true;

}
