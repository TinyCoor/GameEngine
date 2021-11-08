
#include "VulkanSwapChain.h"
#include "VulkanUtils.h"
#include "VulkanApplication.h"
#include "Macro.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanRenderPassBuilder.h"
#include <functional>
#include <limits>
#include <cassert>

VulkanSwapChain::VulkanSwapChain(const VulkanContext* ctx,VkDeviceSize Size)
:context(ctx),uboSize(Size){

}

VulkanSwapChain::~VulkanSwapChain() {
//    shutdownPersistent();
//    shutdown();
}

VulkanSwapChain::SupportedDetails VulkanSwapChain::fetchSwapchainSupportedDetails(VkPhysicalDevice physical_device,
                                                                      VkSurfaceKHR surface) {
    SupportedDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device,surface,&details.capabilities);

    uint32_t  formatCount =0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,surface,&formatCount, nullptr);
    if(formatCount > 0){
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,surface,&formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,surface,&presentModeCount, nullptr);
    if(presentModeCount > 0){
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,surface,&presentModeCount, details.presentModes.data());
    }

    return details;
}

VulkanSwapChain::Settings VulkanSwapChain::selectOptimalSwapchainSettings(SupportedDetails& details,int width,int height) {
    Settings settings{};

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

void VulkanSwapChain::init(int width,int height) {
    initTransient(width,height);
    initPersistent();

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

    vkWaitForFences(context->device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    VkResult result = vkAcquireNextImageKHR(
            context->device,
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
    vkMapMemory(context->device, frame.uniformBuffersMemory, 0, uboSize, 0, &ubo);
    memcpy(ubo, &state, sizeof(RenderState));
    vkUnmapMemory(context->device, frame.uniformBuffersMemory);


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

    //TODO EndCommand Buffer
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

    vkResetFences(context->device, 1, &inFlightFences[currentFrame]);
    //TODO Maybe this is a hardware error, In Surface there is a bug
    VK_CHECK( vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]),"Can't submit command buffer");


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

    auto result = vkQueuePresentKHR(context->presentQueue, &presentInfo);
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
        swapchainDescriptorSetAllocInfo.descriptorPool = context->descriptorPool;
        swapchainDescriptorSetAllocInfo.descriptorSetCount = 1;
        swapchainDescriptorSetAllocInfo.pSetLayouts =&descriptorSetLayout;

        VK_CHECK(vkAllocateDescriptorSets(context->device, &swapchainDescriptorSetAllocInfo, &frame.swapchainDescriptorSet),
                 "Can't allocate swap chain descriptor sets");

        VulkanUtils::bindUniformBuffer(
                context->device,
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

        VK_CHECK(vkCreateFramebuffer(context->device, &framebufferInfo, nullptr, &frame.frameBuffer),"Can't create framebuffer");

        // Create command buffers
        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = context->commandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;

        VK_CHECK(vkAllocateCommandBuffers(context->device, &allocateInfo, &frame.commandBuffer),"Can't create command buffers");

        i++;
    }

}

void VulkanSwapChain::shutdownFrames() {

    for (int i = 0; i <frames.size() ; ++i) {
        vkDestroyFramebuffer(context->device,frames[i].frameBuffer, nullptr);
        vkDestroyBuffer(context->device, frames[i].uniformBuffers, nullptr);
        vkFreeMemory(context->device, frames[i].uniformBuffersMemory, nullptr);
        vkFreeCommandBuffers(context->device, context->commandPool, 1, &frames[i].commandBuffer);
        vkFreeDescriptorSets(context->device,context->descriptorPool,1,&frames[i].swapchainDescriptorSet);
        frames[i].commandBuffer =VK_NULL_HANDLE;
        frames[i].uniformBuffers =VK_NULL_HANDLE;
        frames[i].uniformBuffersMemory = VK_NULL_HANDLE;
        frames[i].commandBuffer =VK_NULL_HANDLE;
        frames[i].swapchainDescriptorSet =VK_NULL_HANDLE;
    }
    frames.clear();

}

void VulkanSwapChain::initPersistent() {

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
        if (vkCreateSemaphore(context->device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context->device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context->device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
    //create descriptor set layout and render pass
    VulkanDescriptorSetLayoutBuilder swapchainDescriptorSetLayoutBuilder(context);
    descriptorSetLayout = swapchainDescriptorSetLayoutBuilder
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL)
            .build();

    VulkanRenderPassBuilder renderPassBuilder(context);
    renderPass =  renderPassBuilder.addColorAttachment(swapChainImageFormat, context->maxMSAASamples,
                                         VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_STORE)
                .addColorResolveAttachment(swapChainImageFormat,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,VK_ATTACHMENT_STORE_OP_STORE)
                .addDepthStencilAttachment(depthFormat, context->maxMSAASamples,VK_ATTACHMENT_LOAD_OP_CLEAR)
                .addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
                .addColorAttachmentReference(0,0)
                .addColorResolveAttachmentReference(0,1)
                .setDepthStencilAttachment(0,2)
                .build();

    VulkanRenderPassBuilder noClearRenderPassBuilder(context);
    noClearRenderPass =  noClearRenderPassBuilder
        .addColorAttachment(swapChainImageFormat, context->maxMSAASamples,
                            VK_ATTACHMENT_LOAD_OP_DONT_CARE,VK_ATTACHMENT_STORE_OP_STORE)
        .addColorResolveAttachment(swapChainImageFormat,
                                   VK_ATTACHMENT_LOAD_OP_DONT_CARE,VK_ATTACHMENT_STORE_OP_STORE)
        .addDepthStencilAttachment(depthFormat, context->maxMSAASamples)
        .addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
        .addColorAttachmentReference(0,0)
        .addColorResolveAttachmentReference(0,1)
        .setDepthStencilAttachment(0,2)
        .build();

}

void VulkanSwapChain::shutdownPersistent() {
    for (int i = 0; i <MAX_FRAME_IN_FLIGHT ; ++i) {
        vkDestroySemaphore(context->device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(context->device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(context->device, inFlightFences[i], nullptr);
    }
    imageAvailableSemaphores.clear();
    renderFinishedSemaphores.clear();
    inFlightFences.clear();

    for (int i = 0; i < swapChainImages.size(); ++i) {
        vkDestroyImage(context->device,swapChainImages[i], nullptr);
        vkDestroyImageView(context->device,swapChainImageViews[i], nullptr);
        swapChainImages[i] =VK_NULL_HANDLE;
        swapChainImageViews[i]= VK_NULL_HANDLE;
    }

    vkDestroyRenderPass(context->device,renderPass, nullptr);
    renderPass = VK_NULL_HANDLE;

    vkDestroyRenderPass(context->device,noClearRenderPass, nullptr);
    noClearRenderPass = VK_NULL_HANDLE;

    vkDestroyDescriptorSetLayout(context->device,descriptorSetLayout, nullptr);
    descriptorSetLayout= VK_NULL_HANDLE;

}

void VulkanSwapChain::shutdownTransient() {
    for (int i = 0; i <swapChainImageViews.size() ; ++i) {
        vkDestroyImageView(context->device,swapChainImageViews[i], nullptr);
    }

    swapChainImageViews.clear();
    swapChainImages.clear();

    vkDestroyImage(context->device,depthImage, nullptr);
    depthImage = VK_NULL_HANDLE;
    vkDestroyImageView(context->device,depthImageView, nullptr);
    depthImageView =  VK_NULL_HANDLE;
    vkFreeMemory(context->device,depthImageMemory, nullptr);
    depthImageMemory = VK_NULL_HANDLE;

    vkDestroyImage(context->device,colorImage, nullptr);
    colorImage = VK_NULL_HANDLE;
    vkDestroyImageView(context->device,colorImageView, nullptr);
    colorImageView =  VK_NULL_HANDLE;
    vkFreeMemory(context->device,colorImageMemory, nullptr);
    colorImageMemory = VK_NULL_HANDLE;

    vkDestroySwapchainKHR(context->device,swapchain, nullptr);
    swapchain= VK_NULL_HANDLE;

}

void VulkanSwapChain::initTransient(int width,int height) {

    swapChainExtent.width = width;
    swapChainExtent.height = height;

    SupportedDetails details = fetchSwapchainSupportedDetails(context->physicalDevice,context->surface);
    Settings settings = selectOptimalSwapchainSettings(details,width,height);

    uint32_t imageCount = details.capabilities.minImageCount + 1;

    if(details.capabilities.maxImageCount > 0 ){
        imageCount = std::min(imageCount,details.capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR swapChainInfo{};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = context->surface;
    swapChainInfo.minImageCount = imageCount;
    swapChainInfo.imageFormat = settings.format.format;
    swapChainInfo.imageColorSpace =settings.format.colorSpace;
    swapChainInfo.imageExtent = settings.extent;
    swapChainInfo.imageArrayLayers= 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if(context->graphicsQueueFamily != context->presentQueueFamily){
        uint32_t queueFamilies[] = {context->graphicsQueueFamily,context->presentQueueFamily};
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

    VK_CHECK(vkCreateSwapchainKHR(context->device,&swapChainInfo, nullptr,&swapchain),
             "failed to created swapchain\n");

    uint32_t  swapChainImageCount =0;
    vkGetSwapchainImagesKHR(context->device,swapchain,&swapChainImageCount, nullptr);
    assert(swapChainImageCount > 0 );

    swapChainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(context->device,swapchain,&swapChainImageCount, swapChainImages.data());

    swapChainImageFormat = settings.format.format;
    swapChainExtent = settings.extent;

    //
    swapChainImageViews.resize(swapChainImageCount);
    for (int i = 0; i <swapChainImageViews.size() ; ++i) {
        swapChainImageViews[i] = VulkanUtils::createImageView(context->device,
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
                               context->maxMSAASamples,
                               swapChainImageFormat,
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               colorImage, colorImageMemory
    );

    colorImageView = VulkanUtils::createImageView(context->device,
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
    depthFormat =VulkanUtils::selectOptimalImageFormat(context->physicalDevice);
    VulkanUtils::createImage2D(context,
                               swapChainExtent.width,
                               swapChainExtent.height,
                               1,
                               context->maxMSAASamples,
                               depthFormat,
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               depthImage, depthImageMemory
    );

    depthImageView = VulkanUtils::createImageView(context->device,
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


