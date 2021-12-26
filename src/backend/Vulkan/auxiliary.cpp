//
// Created by 12132 on 2021/12/26.
//
#include "auxiliary.h"
#include "Device.h"
#include "Utils.h"
#include <cassert>
namespace render::backend::vulkan{

void createTextureData(const Device *context, Texture *texture,
                       Format format, const void *data,
                       int num_data_mipmaps, int num_data_layer)
{
    VkImageUsageFlags usage_flags = Utils::getImageUsageFlags(texture->format);

    Utils::createImage(
        context,
        texture->type,
        texture->width, texture->height, texture->depth,
        texture->num_mipmaps, texture->num_layers,
        texture->samples, texture->format, texture->tiling,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | usage_flags,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        texture->flags,
        texture->image,
        texture->memory
    );

    VkImageLayout source_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (data != nullptr) {
        // prepare for transfer
        Utils::transitionImageLayout(
            context,
            texture->image,
            texture->format,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            0, texture->num_mipmaps,
            0, texture->num_layers
        );

        Utils::fillImage(
            context,
            texture->image,
            texture->width, texture->height, texture->depth,
            texture->num_mipmaps, texture->num_layers,
            Utils::getPixelSize(format),
            texture->format,
            data,
            num_data_mipmaps,
            num_data_layer
        );

        source_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }

    Utils::transitionImageLayout(context, texture->image, texture->format,
                                 source_layout,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                 0, texture->num_mipmaps,
                                 0, texture->num_layers);

    // Create image view & sampler
    texture->view = Utils::createImageView(
        context,
        texture->image,
        texture->format,
        Utils::getImageAspectFlags(texture->format),
        Utils::getImageBaseViewType(texture->type, texture->flags, texture->num_layers),
        0, texture->num_mipmaps,
        0, texture->num_layers
    );

    texture->sampler = Utils::createSampler(context, 0, texture->num_mipmaps);

}

void selectOptimalSwapChainSettings(Device *context, SwapChain *swapchain)
{
    //get surface capability
    auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->PhysicalDevice(),
                                                         swapchain->surface,
                                                         &swapchain->surface_capabilities);
    if (res != VK_SUCCESS) {
        std::cerr << "get surface capability error: %d" << res << std::endl;
    }

    // select best surface format
    uint32_t num_surface_format = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->PhysicalDevice(), swapchain->surface, &num_surface_format, nullptr);
    assert(num_surface_format > 0);
    std::vector<VkSurfaceFormatKHR> surface_formats(num_surface_format);
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->PhysicalDevice(),
                                         swapchain->surface,
                                         &num_surface_format,
                                         surface_formats.data());
    // no preferred format
    if (surface_formats.size() == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED) {
        swapchain->surface_format = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    } else {  // Otherwise, select one of the available formatselse
        swapchain->surface_format = surface_formats[0];
        for (const auto &format: surface_formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                swapchain->surface_format = format;
                break;
            }
        }
    }

    //select best present mode
    uint32_t num_present_mode = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->PhysicalDevice(),
                                              swapchain->surface,
                                              &num_present_mode,
                                              nullptr);
    assert(num_present_mode > 0);
    std::vector<VkPresentModeKHR> present_modes(num_present_mode);
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->PhysicalDevice(),
                                              swapchain->surface,
                                              &num_present_mode,
                                              present_modes.data());
    swapchain->present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto &presentMode: present_modes) {
        // Some drivers currently don't properly support FIFO present mode,
        // so we should prefer IMMEDIATE mode if MAILBOX mode is not available
        if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            swapchain->present_mode = presentMode;

        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            swapchain->present_mode = presentMode;
            break;
        }
    }

    const VkSurfaceCapabilitiesKHR &capabilities = swapchain->surface_capabilities;
    // Select current swap extent if window manager doesn't allow to set custom extent
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        swapchain->sizes = capabilities.currentExtent;
    } else {  // Otherwise, manually set extent to match the min/max extent bounds

        swapchain->sizes.width = std::clamp(
            swapchain->sizes.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width
        );

        swapchain->sizes.height = std::clamp(
            swapchain->sizes.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height
        );
    }

    swapchain->num_images = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0) {
        swapchain->num_images = std::min(swapchain->num_images, capabilities.maxImageCount);
    }

}

bool createSwapchainObjects(Device *context,
                            SwapChain *swapchain,
                            uint32_t width,
                            uint32_t height)
{

    const VkSurfaceCapabilitiesKHR &capabilities = swapchain->surface_capabilities;
    VkSwapchainCreateInfoKHR swapChainInfo{};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = swapchain->surface;
    swapChainInfo.minImageCount = swapchain->num_images;
    swapChainInfo.imageFormat = swapchain->surface_format.format; // settings.format.format;
    swapChainInfo.imageColorSpace = swapchain->surface_format.colorSpace;    //settings.format.colorSpace;
    swapChainInfo.imageExtent = swapchain->sizes;//  swapChainExtent;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (context->GraphicsQueueFamily() != swapchain->present_queue_family) {
        uint32_t queueFamilies[] = {context->GraphicsQueueFamily(), swapchain->present_queue_family};
        swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainInfo.queueFamilyIndexCount = 2;
        swapChainInfo.pQueueFamilyIndices = queueFamilies;

    } else {
        swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainInfo.queueFamilyIndexCount = 0;
        swapChainInfo.pQueueFamilyIndices = nullptr;
    }

    swapChainInfo.preTransform = capabilities.currentTransform;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.presentMode = swapchain->present_mode; // settings.presentMode;
    swapChainInfo.clipped = VK_TRUE;
    swapChainInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(context->LogicDevice(), &swapChainInfo, nullptr, &swapchain->swap_chain) != VK_SUCCESS) {
        std::cerr << "vulkan::createTransientSwapChainObjects(): vkCreateSwapchainKHR failed" << std::endl;
        return false;
    }

    vkGetSwapchainImagesKHR(context->LogicDevice(), swapchain->swap_chain, &swapchain->num_images, nullptr);
    assert(swapchain->num_images > 0 && swapchain->num_images < render::backend::vulkan::SwapChain::MAX_IMAGES);
    vkGetSwapchainImagesKHR(context->LogicDevice(), swapchain->swap_chain, &swapchain->num_images, swapchain->images);

    for (int i = 0; i < swapchain->num_images; ++i) {
        swapchain->views[i] = Utils::createImageView(context,
                                                     swapchain->images[i],
                                                     swapchain->surface_format.format,
                                                     VK_IMAGE_ASPECT_COLOR_BIT,
                                                     VK_IMAGE_VIEW_TYPE_2D);
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        if (vkCreateSemaphore(context->LogicDevice(), &semaphoreInfo, nullptr, &swapchain->image_available_gpu[i])
            != VK_SUCCESS) {
            std::cerr << "failed to create semaphores!" << std::endl;
            // destroySwapChain(swapchain);
            return false;
        }
    }

    return true;
}

void destroySwapchainObjects(Device *context, SwapChain *swapchain)
{
    for (size_t i = 0; i < swapchain->num_images; ++i) {
        vkDestroyImageView(context->LogicDevice(), swapchain->views[i], nullptr);
        swapchain->views[i] = VK_NULL_HANDLE;
        swapchain->images[i] = VK_NULL_HANDLE;
        vkDestroySemaphore(context->LogicDevice(),swapchain->image_available_gpu[i], nullptr);
        swapchain->image_available_gpu[i] =VK_NULL_HANDLE;
    }
    vkDestroySwapchainKHR(context->LogicDevice(), swapchain->swap_chain, nullptr);
    swapchain->swap_chain = VK_NULL_HANDLE;
}

void updateBindSetLayout(const Device *device, BindSet *bind_set, VkDescriptorSetLayout new_layout)
{
    if (new_layout == bind_set->set_layout)
        return;

    bind_set->set_layout = new_layout;

    if (bind_set->set != VK_NULL_HANDLE)
        vkFreeDescriptorSets(device->LogicDevice(), device->DescriptorPool(), 1, &bind_set->set);

    VkDescriptorSetAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorPool = device->DescriptorPool();
    info.descriptorSetCount = 1;
    info.pSetLayouts = &new_layout;

    vkAllocateDescriptorSets(device->LogicDevice(), &info, &bind_set->set);
    assert(bind_set->set);

    for (uint8_t i = 0; i < vulkan::BindSet::MAX_BINDINGS; ++i)
    {
        if (!bind_set->binding_used[i])
            continue;

        bind_set->binding_dirty[i] = true;
    }
}
}