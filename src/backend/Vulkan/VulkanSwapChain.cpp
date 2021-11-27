
#include "VulkanSwapChain.h"
#include "VulkanUtils.h"
#include "../../app/VulkanApplication.h"
#include "Macro.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanRenderPassBuilder.h"
#include <functional>
#include <GLFW/glfw3.h>
#include <limits>
#include <cassert>

using namespace render::backend::vulkan;

VulkanSwapChain::VulkanSwapChain(const VulkanContext* ctx,void* navWindow,VkDeviceSize Size)
    : context(ctx),
      nativeWindow(navWindow),
      uboSize(Size){}

VulkanSwapChain::~VulkanSwapChain() {
}

void VulkanSwapChain::init(int width, int height) {
    initPersistent();
    initTransient(width,height);
    initFrames(uboSize);

}

void VulkanSwapChain::reinit(int width,int height) {

    shutdownTransient();
    shutdownFrames();

    initTransient(width,height);
    initFrames(uboSize);

}

void VulkanSwapChain::shutdown() {
    shutdownTransient();
    shutdownFrames();
    shutdownPersistent();
}

bool VulkanSwapChain::Acquire(const RenderState& state,VulkanRenderFrame& frame) {

    vkWaitForFences(context->Device(), 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    VkResult result = vkAcquireNextImageKHR(
            context->Device(),
            swapchain,
            std::numeric_limits<uint64_t>::max(),
            imageAvailableSemaphores[currentFrame],
            VK_NULL_HANDLE,
            &imageIndex
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        return false;
    }

    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Can't aquire swap chain image");

    frame = frames[imageIndex];

    //Copy Render State to ubo
    void *ubo = nullptr;
    vkMapMemory(context->Device(), frame.uniformBuffersMemory, 0, uboSize, 0, &ubo);
    memcpy(ubo, &state, sizeof(RenderState));
    vkUnmapMemory(context->Device(), frame.uniformBuffersMemory);


    //reset command buffer
    VK_CHECK(vkResetCommandBuffer(frame.commandBuffer,0),"Can't Reset Command Buffer");
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional

    VK_CHECK(vkBeginCommandBuffer(frame.commandBuffer, &beginInfo),"Can't begin recording command buffer");

    return true;
}

bool VulkanSwapChain::Present( VulkanRenderFrame& frame) {
    VK_CHECK(vkEndCommandBuffer(frame.commandBuffer),"Can't record command buffer");

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frame.commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(context->Device(), 1, &inFlightFences[currentFrame]);
    VK_CHECK( vkQueueSubmit(context->GraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]),
              "Can't submit command buffer");


    VkSwapchainKHR swapChains[] = {swapchain};
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    VulkanUtils::transitionImageLayout(
            context,
            swapChainImages[imageIndex],
            swapChainImageFormat,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    );

    auto result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR )
        return false;
    else if (result != VK_SUCCESS)
        throw std::runtime_error("Can't aquire swap chain image");

    currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
    return true;
}

void VulkanSwapChain::initFrames(VkDeviceSize uboSize) {

    uint32_t imageCount = static_cast<uint32_t>(swapChainImages.size());
    frames.resize(imageCount);
    int i =0;
    for (auto& frame: frames) {
        //create Uniform Buffer Object
        VulkanUtils::createBuffer(
                context,
                uboSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                frame.uniformBuffers,
                frame.uniformBuffersMemory
        );
         //Create descriptor Set
        VkDescriptorSetAllocateInfo swapchainDescriptorSetAllocInfo = {};
        swapchainDescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        swapchainDescriptorSetAllocInfo.descriptorPool = context->DescriptorPool();
        swapchainDescriptorSetAllocInfo.descriptorSetCount = 1;
        swapchainDescriptorSetAllocInfo.pSetLayouts =&descriptorSetLayout;

        VK_CHECK(vkAllocateDescriptorSets(context->Device(),
                                          &swapchainDescriptorSetAllocInfo,
                                          &frame.swapchainDescriptorSet),
                 "Can't allocate swap chain descriptor sets");

        VulkanUtils::bindUniformBuffer(
                context->Device(),
                frame.swapchainDescriptorSet,
                0,
                frame.uniformBuffers,
                0,
                sizeof(RenderState)
        );
        std::array<VkImageView, 3> attachments = {
                colorImageView,
                swapChainImageViews[i],
                depthImageView,
        };
        //create Frame buffer
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        VK_CHECK(vkCreateFramebuffer(context->Device(), &framebufferInfo, nullptr, &frame.frameBuffer),"Can't create framebuffer");

        // Create command buffers
        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = context->CommandPool();
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;

        VK_CHECK(vkAllocateCommandBuffers(context->Device(), &allocateInfo, &frame.commandBuffer),"Can't create command buffers");

        i++;
    }

}

void VulkanSwapChain::shutdownFrames() {

    for (int i = 0; i <frames.size() ; ++i) {
        vkDestroyFramebuffer(context->Device(),frames[i].frameBuffer, nullptr);
        vkDestroyBuffer(context->Device(), frames[i].uniformBuffers, nullptr);
        vkFreeMemory(context->Device(), frames[i].uniformBuffersMemory, nullptr);
        vkFreeCommandBuffers(context->Device(), context->CommandPool(), 1, &frames[i].commandBuffer);
        vkFreeDescriptorSets(context->Device(),context->DescriptorPool(),1,&frames[i].swapchainDescriptorSet);
        frames[i].commandBuffer =VK_NULL_HANDLE;
        frames[i].uniformBuffers =VK_NULL_HANDLE;
        frames[i].frameBuffer =VK_NULL_HANDLE;
        frames[i].uniformBuffersMemory = VK_NULL_HANDLE;
        frames[i].commandBuffer =VK_NULL_HANDLE;
        frames[i].swapchainDescriptorSet =VK_NULL_HANDLE;
    }
    frames.clear();

}

void VulkanSwapChain::initPersistent() {
    //TODO support ios android linux platform
    VK_CHECK( glfwCreateWindowSurface(context->Instance(),(GLFWwindow*)nativeWindow, nullptr,&surface),"Create Surface failed");

    // Fetch present queue
    presentQueueFamily = VulkanUtils::fetchPresentQueueFamily(
            context->PhysicalDevice(),
            surface,
            context->GraphicsQueueFamily()
    );

    // Get present queue
    vkGetDeviceQueue(context->Device(), presentQueueFamily, 0, &presentQueue);
    if (presentQueue == VK_NULL_HANDLE)
        throw std::runtime_error("Can't get present queue from logical device");

    // Select optimal swap chain settings
    details = fetchSwapChainSupportDetails(context->PhysicalDevice(), surface);
    settings = selectOptimalSwapChainSettings(details);

    depthFormat = VulkanUtils::selectOptimalImageFormat(context->PhysicalDevice());
    swapChainImageFormat = settings.format.format;
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
        if (vkCreateSemaphore(context->Device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context->Device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context->Device(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
    //create descriptor set layout and render pass
    VulkanDescriptorSetLayoutBuilder swapchainDescriptorSetLayoutBuilder(context);
    descriptorSetLayout = swapchainDescriptorSetLayoutBuilder
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL)
            .build();

    VulkanRenderPassBuilder renderPassBuilder(context);
    renderPass =  renderPassBuilder.addColorAttachment(swapChainImageFormat, context->MaxMSAASamples(),
                                         VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_STORE)
                .addColorResolveAttachment(swapChainImageFormat,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,VK_ATTACHMENT_STORE_OP_STORE)
                .addDepthStencilAttachment(depthFormat, context->MaxMSAASamples(),VK_ATTACHMENT_LOAD_OP_CLEAR)
                .addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
                .addColorAttachmentReference(0,0)
                .addColorResolveAttachmentReference(0,1)
                .setDepthStencilAttachment(0,2)
                .build();

    VulkanRenderPassBuilder noClearRenderPassBuilder(context);
    noClearRenderPass =  noClearRenderPassBuilder
        .addColorAttachment(swapChainImageFormat, context->MaxMSAASamples(),
                            VK_ATTACHMENT_LOAD_OP_DONT_CARE,VK_ATTACHMENT_STORE_OP_STORE)
        .addColorResolveAttachment(swapChainImageFormat,
                                   VK_ATTACHMENT_LOAD_OP_DONT_CARE,VK_ATTACHMENT_STORE_OP_STORE)
        .addDepthStencilAttachment(depthFormat, context->MaxMSAASamples())
        .addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
        .addColorAttachmentReference(0,0)
        .addColorResolveAttachmentReference(0,1)
        .setDepthStencilAttachment(0,2)
        .build();

}

void VulkanSwapChain::shutdownPersistent() {
    for (int i = 0; i <MAX_FRAME_IN_FLIGHT ; ++i) {
        vkDestroySemaphore(context->Device(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(context->Device(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(context->Device(), inFlightFences[i], nullptr);
    }
    imageAvailableSemaphores.clear();
    renderFinishedSemaphores.clear();
    inFlightFences.clear();

    for (int i = 0; i < swapChainImages.size(); ++i) {
        vkDestroyImage(context->Device(),swapChainImages[i], nullptr);
        vkDestroyImageView(context->Device(),swapChainImageViews[i], nullptr);
        swapChainImages[i] =VK_NULL_HANDLE;
        swapChainImageViews[i]= VK_NULL_HANDLE;
    }

    vkDestroyRenderPass(context->Device(),renderPass, nullptr);
    renderPass = VK_NULL_HANDLE;

    vkDestroyRenderPass(context->Device(),noClearRenderPass, nullptr);
    noClearRenderPass = VK_NULL_HANDLE;

    vkDestroyDescriptorSetLayout(context->Device(),descriptorSetLayout, nullptr);
    descriptorSetLayout= VK_NULL_HANDLE;



}

void VulkanSwapChain::shutdownTransient() {
    for (int i = 0; i <swapChainImageViews.size() ; ++i) {
        vkDestroyImageView(context->Device(),swapChainImageViews[i], nullptr);
      //  vkDestroyImage(context->device,swapChainImages[i], nullptr);
    }

    swapChainImageViews.clear();
    swapChainImages.clear();

    vkDestroyImage(context->Device(),depthImage, nullptr);
    depthImage = VK_NULL_HANDLE;
    vkDestroyImageView(context->Device(),depthImageView, nullptr);
    depthImageView =  VK_NULL_HANDLE;
    vkFreeMemory(context->Device(),depthImageMemory, nullptr);
    depthImageMemory = VK_NULL_HANDLE;

    vkDestroyImage(context->Device(),colorImage, nullptr);
    colorImage = VK_NULL_HANDLE;
    vkDestroyImageView(context->Device(),colorImageView, nullptr);
    colorImageView =  VK_NULL_HANDLE;
    vkFreeMemory(context->Device(),colorImageMemory, nullptr);
    colorImageMemory = VK_NULL_HANDLE;

    vkDestroySwapchainKHR(context->Device(),swapchain, nullptr);
    swapchain= VK_NULL_HANDLE;

}

void VulkanSwapChain::initTransient(int width,int height) {

    // Select current swap extent if window manager doesn't allow to set custom extent
    if (details.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        swapChainExtent = details.capabilities.currentExtent;
    }
        // Otherwise, manually set extent to match the min/max extent bounds
    else
    {
        const VkSurfaceCapabilitiesKHR &capabilities = details.capabilities;

        swapChainExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
        };

        swapChainExtent.width = std::clamp(
                swapChainExtent.width,
                capabilities.minImageExtent.width,
                capabilities.maxImageExtent.width
        );

        swapChainExtent.height = std::clamp(
                swapChainExtent.height,
                capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height
        );
    }


    uint32_t imageCount = details.capabilities.minImageCount + 1;

    if(details.capabilities.maxImageCount > 0 ){
        imageCount = std::min(imageCount,details.capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR swapChainInfo{};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = surface;
    swapChainInfo.minImageCount = imageCount;
    swapChainInfo.imageFormat = settings.format.format;
    swapChainInfo.imageColorSpace =settings.format.colorSpace;
    swapChainInfo.imageExtent = swapChainExtent;
    swapChainInfo.imageArrayLayers= 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if(context->GraphicsQueueFamily() != presentQueueFamily){
        uint32_t queueFamilies[] = {context->GraphicsQueueFamily(),presentQueueFamily};
        swapChainInfo.imageSharingMode =VK_SHARING_MODE_CONCURRENT;
        swapChainInfo.queueFamilyIndexCount = 2;
        swapChainInfo.pQueueFamilyIndices = queueFamilies;

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

    VK_CHECK(vkCreateSwapchainKHR(context->Device(),&swapChainInfo, nullptr,&swapchain),
             "failed to created swapchain\n");

    uint32_t  swapChainImageCount =0;
    vkGetSwapchainImagesKHR(context->Device(),swapchain,&swapChainImageCount, nullptr);
    assert(swapChainImageCount > 0 );

    swapChainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(context->Device(),swapchain,&swapChainImageCount, swapChainImages.data());


    //
    swapChainImageViews.resize(swapChainImageCount);
    for (int i = 0; i <swapChainImageViews.size() ; ++i) {
        swapChainImageViews[i] = VulkanUtils::createImageView(context->Device(),
                                                              swapChainImages[i],
                                                              swapChainImageFormat,
                                                              VK_IMAGE_ASPECT_COLOR_BIT,
                                                              VK_IMAGE_VIEW_TYPE_2D);
    }

    //Create Color Image ImageView
    VulkanUtils::createImage2D(context,
                               swapChainExtent.width,
                               swapChainExtent.height,
                               1,
                               context->MaxMSAASamples(),
                               swapChainImageFormat,
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               colorImage, colorImageMemory
    );

    colorImageView = VulkanUtils::createImageView(context->Device(),
                                                  colorImage,
                                                  swapChainImageFormat,
                                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                                  VK_IMAGE_VIEW_TYPE_2D);
    VulkanUtils::transitionImageLayout(context,
                                       colorImage,
                                       swapChainImageFormat,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    //Create Depth Buffer
    depthFormat =VulkanUtils::selectOptimalImageFormat(context->PhysicalDevice());
    VulkanUtils::createImage2D(context,
                               swapChainExtent.width,
                               swapChainExtent.height,
                               1,
                               context->MaxMSAASamples(),
                               depthFormat,
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               depthImage, depthImageMemory
    );

    depthImageView = VulkanUtils::createImageView(context->Device(),
                                                  depthImage,
                                                  depthFormat,
                                                  VK_IMAGE_ASPECT_DEPTH_BIT,
                                                  VK_IMAGE_VIEW_TYPE_2D);

    VulkanUtils::transitionImageLayout(context,
                                       depthImage,
                                       depthFormat,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}

VulkanSwapChain::Settings
VulkanSwapChain::selectOptimalSwapChainSettings(const VulkanSwapChain::SupportDetails &details) const {
    assert(!details.formats.empty());
    assert(!details.presentModes.empty());

    VulkanSwapChain::Settings result;

    // Select the best format if the surface has no preferred format
    if (details.formats.size() == 1 && details.formats[0].format == VK_FORMAT_UNDEFINED)
    {
        result.format = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }
        // Otherwise, select one of the available formats
    else
    {
        result.format = details.formats[0];
        for (const auto &format : details.formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                result.format = format;
                break;
            }
        }
    }

    // Select the best present mode
    result.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto &presentMode : details.presentModes)
    {
        // Some drivers currently don't properly support FIFO present mode,
        // so we should prefer IMMEDIATE mode if MAILBOX mode is not available
        if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            result.presentMode = presentMode;

        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            result.presentMode = presentMode;
            break;
        }
    }

    return result;
}

VulkanSwapChain::SupportDetails
VulkanSwapChain::fetchSwapChainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const {

    SupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice,surface,&details.capabilities);

    uint32_t  formatCount =0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,surface,&formatCount, nullptr);
    if(formatCount > 0){
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,surface,&formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,surface,&presentModeCount, nullptr);
    if(presentModeCount > 0){
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,surface,&presentModeCount, details.presentModes.data());
    }

    return details;
}


