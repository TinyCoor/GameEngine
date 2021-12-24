//
// Created by 12132 on 2021/12/14.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_AUXILIARY_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_AUXILIARY_H
#include <volk.h>
#include "../driver.h"
#include "VulkanUtils.h"
#include <iostream>
#include <functional>
#include <cstdint>
#include <limits>

namespace render::backend::vulkan{

template <class T>
static void hash_combine(uint64_t &s, const T &v)
{
    std::hash<T> h;
    s^= h(v) + 0x9e3779b9 + (s<< 6) + (s>> 2);
}

static VkShaderStageFlagBits toShaderStage(ShaderType type){
    static VkShaderStageFlagBits support_stages[static_cast<uint32_t>(ShaderType::MAX)] ={
        // Graphics Pipeline
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        VK_SHADER_STAGE_GEOMETRY_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        ///Compute pipeline
        VK_SHADER_STAGE_COMPUTE_BIT,

        // RayTracing
        VK_SHADER_STAGE_RAYGEN_BIT_NV,
        VK_SHADER_STAGE_INTERSECTION_BIT_NV,
        VK_SHADER_STAGE_ANY_HIT_BIT_NV,
        VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV,
        VK_SHADER_STAGE_MISS_BIT_NV,
        VK_SHADER_STAGE_CALLABLE_BIT_NV,



    };
    return support_stages[static_cast<uint32_t>(type)];
}

inline VkCullModeFlags toCullMode(CullMode cull_mode){
    static VkCullModeFlags modes[static_cast<uint32_t>(CullMode::MAX)] ={
        VK_CULL_MODE_NONE,
        VK_CULL_MODE_FRONT_BIT,
        VK_CULL_MODE_BACK_BIT,
        VK_CULL_MODE_FRONT_AND_BACK,
    };
    return modes[static_cast<uint32_t>(cull_mode)];
}

inline VkCompareOp toDepthCompareFunc(DepthCompareFunc depth_compare_func){
    static VkCompareOp depth_funcs[static_cast<uint32_t>( DepthCompareFunc::MAX)] ={
        VK_COMPARE_OP_NEVER,
        VK_COMPARE_OP_LESS,
        VK_COMPARE_OP_EQUAL,
        VK_COMPARE_OP_LESS_OR_EQUAL,
        VK_COMPARE_OP_GREATER,
        VK_COMPARE_OP_NOT_EQUAL,
        VK_COMPARE_OP_GREATER_OR_EQUAL,
        VK_COMPARE_OP_ALWAYS
    };
    return depth_funcs[static_cast<uint32_t>(depth_compare_func)];
}

inline VkBlendFactor toBlendFactor(BlendFactor blend_factor){
    static VkBlendFactor factors[static_cast<uint32_t>(BlendFactor::MAX)] ={
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_SRC_COLOR,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
        VK_BLEND_FACTOR_DST_COLOR,
        VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
        VK_BLEND_FACTOR_SRC_ALPHA ,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_FACTOR_DST_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    };
    return factors[static_cast<uint32_t>(blend_factor)];
}

static VkFormat toVkFormat(Format format)
{
    static VkFormat supported_formats[static_cast<int>(Format::MAX)] =
        {
            VK_FORMAT_UNDEFINED,

            // 8-bit formats
            VK_FORMAT_R8_UNORM, VK_FORMAT_R8_SNORM, VK_FORMAT_R8_UINT, VK_FORMAT_R8_SINT,
            VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R8G8_UINT, VK_FORMAT_R8G8_SINT,
            VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8_SNORM, VK_FORMAT_R8G8B8_UINT, VK_FORMAT_R8G8B8_SINT,
            VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_B8G8R8_SNORM, VK_FORMAT_B8G8R8_UINT, VK_FORMAT_B8G8R8_SINT,
            VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_UINT, VK_FORMAT_R8G8B8A8_SINT,
            VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM, VK_FORMAT_B8G8R8A8_UINT, VK_FORMAT_B8G8R8A8_SINT,

            // 16-bit formats
            VK_FORMAT_R16_UNORM, VK_FORMAT_R16_SNORM, VK_FORMAT_R16_UINT, VK_FORMAT_R16_SINT, VK_FORMAT_R16_SFLOAT,
            VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16_UINT, VK_FORMAT_R16G16_SINT,
            VK_FORMAT_R16G16_SFLOAT,
            VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R16G16B16_UINT, VK_FORMAT_R16G16B16_SINT,
            VK_FORMAT_R16G16B16_SFLOAT,
            VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R16G16B16A16_UINT,
            VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SFLOAT,

            // 32-bit formats
            VK_FORMAT_R32_UINT, VK_FORMAT_R32_SINT, VK_FORMAT_R32_SFLOAT,
            VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32_SFLOAT,
            VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_SINT, VK_FORMAT_R32G32B32A32_SFLOAT,

            // depth formats
            VK_FORMAT_D16_UNORM,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
        };

    return supported_formats[static_cast<int>(format)];
}

static VkIndexType ToVkIndexType(IndexSize index_size)
{
    static VkIndexType supports[static_cast<unsigned int>(IndexSize::MAX)] = {
        VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32
    };
    return supports[static_cast<uint32_t>(index_size)];
}

static VkPrimitiveTopology toPrimitiveTopology(RenderPrimitiveType type)
{
    static VkPrimitiveTopology supports_type[static_cast<uint32_t>(RenderPrimitiveType::MAX)] = {
        // points
        VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        // lines
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
        VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,

        //traiangles
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,

        // patches
        VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
    };
    return supports_type[static_cast<uint32_t>(type)];
}

static size_t toPixelSize(Format format)
{
    static uint8_t supported_formats[static_cast<int>(Format::MAX)] =
        {
            0,

            // 8-bit formats
            1, 1, 1, 1,
            2, 2, 2, 2,
            3, 3, 3, 3,
            3, 3, 3, 3,
            4, 4, 4, 4,
            4, 4, 4, 4,

            // 16-bit formats
            2, 2, 2, 2, 2,
            4, 4, 4, 4, 4,
            6, 6, 6, 6, 6,
            8, 8, 8, 8, 8,

            // 32-bit formats
            4, 4, 4,
            8, 8, 8,
            12, 12, 12,
            16, 16, 16,

            // depth formats
            2, 3, 4, 4, 5,
        };

    return supported_formats[static_cast<int>(format)];
}

static uint8_t toIndexSize(IndexSize size)
{
    static uint8_t supported_sizes[static_cast<int>(IndexSize::MAX)] =
        {
            2, 4
        };

    return supported_sizes[static_cast<int>(size)];
}

static VkImageAspectFlags toImageAspectFlags(VkFormat format)
{
    if (format == VK_FORMAT_UNDEFINED)
        return 0;

    switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT: return VK_IMAGE_ASPECT_DEPTH_BIT;
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    return VK_IMAGE_ASPECT_COLOR_BIT;
}

static Format fromFormat(VkFormat format)
{
    switch (format) {
    case VK_FORMAT_UNDEFINED: return Format::UNDEFINED;

    case VK_FORMAT_R8_UNORM: return Format::R8_UNORM;
    case VK_FORMAT_R8_SNORM: return Format::R8_SNORM;
    case VK_FORMAT_R8_UINT: return Format::R8_UINT;
    case VK_FORMAT_R8_SINT: return Format::R8_SINT;
    case VK_FORMAT_R8G8_UNORM: return Format::R8G8_UNORM;
    case VK_FORMAT_R8G8_SNORM: return Format::R8G8_SNORM;
    case VK_FORMAT_R8G8_UINT: return Format::R8G8_UINT;
    case VK_FORMAT_R8G8_SINT: return Format::R8G8_SINT;
    case VK_FORMAT_R8G8B8_UNORM: return Format::R8G8B8_UNORM;
    case VK_FORMAT_R8G8B8_SNORM: return Format::R8G8B8_SNORM;
    case VK_FORMAT_R8G8B8_UINT: return Format::R8G8B8_UINT;
    case VK_FORMAT_R8G8B8_SINT: return Format::R8G8B8_SINT;
    case VK_FORMAT_B8G8R8_UNORM: return Format::B8G8R8_UNORM;
    case VK_FORMAT_B8G8R8_SNORM: return Format::B8G8R8_SNORM;
    case VK_FORMAT_B8G8R8_UINT: return Format::B8G8R8_UINT;
    case VK_FORMAT_B8G8R8_SINT: return Format::B8G8R8_SINT;
    case VK_FORMAT_R8G8B8A8_UNORM: return Format::R8G8B8A8_UNORM;
    case VK_FORMAT_R8G8B8A8_SNORM: return Format::R8G8B8A8_SNORM;
    case VK_FORMAT_R8G8B8A8_UINT: return Format::R8G8B8A8_UINT;
    case VK_FORMAT_R8G8B8A8_SINT: return Format::R8G8B8A8_SINT;
    case VK_FORMAT_B8G8R8A8_UNORM: return Format::B8G8R8A8_UNORM;
    case VK_FORMAT_B8G8R8A8_SNORM: return Format::B8G8R8A8_SNORM;
    case VK_FORMAT_B8G8R8A8_UINT: return Format::B8G8R8A8_UINT;
    case VK_FORMAT_B8G8R8A8_SINT: return Format::B8G8R8A8_SINT;

    case VK_FORMAT_R16_UNORM: return Format::R16_UNORM;
    case VK_FORMAT_R16_SNORM: return Format::R16_SNORM;
    case VK_FORMAT_R16_UINT: return Format::R16_UINT;
    case VK_FORMAT_R16_SINT: return Format::R16_SINT;
    case VK_FORMAT_R16_SFLOAT: return Format::R16_SFLOAT;
    case VK_FORMAT_R16G16_UNORM: return Format::R16G16_UNORM;
    case VK_FORMAT_R16G16_SNORM: return Format::R16G16_SNORM;
    case VK_FORMAT_R16G16_UINT: return Format::R16G16_UINT;
    case VK_FORMAT_R16G16_SINT: return Format::R16G16_SINT;
    case VK_FORMAT_R16G16_SFLOAT: return Format::R16G16_SFLOAT;
    case VK_FORMAT_R16G16B16_UNORM: return Format::R16G16B16_UNORM;
    case VK_FORMAT_R16G16B16_SNORM: return Format::R16G16B16_SNORM;
    case VK_FORMAT_R16G16B16_UINT: return Format::R16G16B16_UINT;
    case VK_FORMAT_R16G16B16_SINT: return Format::R16G16B16_SINT;
    case VK_FORMAT_R16G16B16_SFLOAT: return Format::R16G16B16_SFLOAT;
    case VK_FORMAT_R16G16B16A16_UNORM: return Format::R16G16B16A16_UNORM;
    case VK_FORMAT_R16G16B16A16_SNORM: return Format::R16G16B16A16_SNORM;
    case VK_FORMAT_R16G16B16A16_UINT: return Format::R16G16B16A16_UINT;
    case VK_FORMAT_R16G16B16A16_SINT: return Format::R16G16B16A16_SINT;
    case VK_FORMAT_R16G16B16A16_SFLOAT: return Format::R16G16B16A16_SFLOAT;

    case VK_FORMAT_R32_UINT: return Format::R32_UINT;
    case VK_FORMAT_R32_SINT: return Format::R32_SINT;
    case VK_FORMAT_R32_SFLOAT: return Format::R32_SFLOAT;
    case VK_FORMAT_R32G32_UINT: return Format::R32G32_UINT;
    case VK_FORMAT_R32G32_SINT: return Format::R32G32_SINT;
    case VK_FORMAT_R32G32_SFLOAT: return Format::R32G32_SFLOAT;
    case VK_FORMAT_R32G32B32_UINT: return Format::R32G32B32_UINT;
    case VK_FORMAT_R32G32B32_SINT: return Format::R32G32B32_SINT;
    case VK_FORMAT_R32G32B32_SFLOAT: return Format::R32G32B32_SFLOAT;
    case VK_FORMAT_R32G32B32A32_UINT: return Format::R32G32B32A32_UINT;
    case VK_FORMAT_R32G32B32A32_SINT: return Format::R32G32B32A32_SINT;
    case VK_FORMAT_R32G32B32A32_SFLOAT: return Format::R32G32B32A32_SFLOAT;

    case VK_FORMAT_D16_UNORM: return Format::D16_UNORM;
    case VK_FORMAT_D16_UNORM_S8_UINT: return Format::D16_UNORM_S8_UINT;
    case VK_FORMAT_D24_UNORM_S8_UINT: return Format::D24_UNORM_S8_UINT;
    case VK_FORMAT_D32_SFLOAT: return Format::D32_SFLOAT;
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return Format::D32_SFLOAT_S8_UINT;

    default: {
       // std::cerr << "vulkan::fromFormat(): unsupported format " << format << std::endl;
        return Format::UNDEFINED;
    }
    }
}

static VkSampleCountFlagBits toSamples(Multisample samples)
{
    static VkSampleCountFlagBits supported_samples[static_cast<int>(Multisample::MAX)] =
        {
            VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_2_BIT,
            VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_8_BIT,
            VK_SAMPLE_COUNT_16_BIT, VK_SAMPLE_COUNT_32_BIT,
            VK_SAMPLE_COUNT_64_BIT,
        };

    return supported_samples[static_cast<int>(samples)];
}

static Multisample fromSamples(VkSampleCountFlagBits samples)
{
    if (samples & VK_SAMPLE_COUNT_64_BIT) {
        return Multisample::COUNT_64;
    }
    if (samples & VK_SAMPLE_COUNT_32_BIT) {
        return Multisample::COUNT_32;
    }
    if (samples & VK_SAMPLE_COUNT_16_BIT) {
        return Multisample::COUNT_16;
    }
    if (samples & VK_SAMPLE_COUNT_8_BIT) {
        return Multisample::COUNT_8;
    }
    if (samples & VK_SAMPLE_COUNT_4_BIT) {
        return Multisample::COUNT_4;
    }
    if (samples & VK_SAMPLE_COUNT_2_BIT) {
        return Multisample::COUNT_2;
    }

    return Multisample::COUNT_1;
}

static VkImageUsageFlags toImageUsageFlags(VkFormat format)
{
    if (format == VK_FORMAT_UNDEFINED)
        return 0;

    switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
}

static VkImageViewType toImageBaseViewType(VkImageType type, VkImageCreateFlags flags, uint32_t num_layers)
{
    if ((type == VK_IMAGE_TYPE_2D) && (num_layers == 1) && (flags == 0))
        return VK_IMAGE_VIEW_TYPE_2D;

    if ((type == VK_IMAGE_TYPE_2D) && (num_layers > 1) && (flags == 0))
        return VK_IMAGE_VIEW_TYPE_2D_ARRAY;

    if ((type == VK_IMAGE_TYPE_3D) && (num_layers == 1) && (flags == 0))
        return VK_IMAGE_VIEW_TYPE_3D;

    if (type == VK_IMAGE_TYPE_2D && num_layers == 6 && (flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT))
        return VK_IMAGE_VIEW_TYPE_CUBE;

    return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
}

static VkCommandBufferLevel toVKCommandBufferLevel(CommandBufferType type)
{
    switch (type) {
    case CommandBufferType::PRIMARY :return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    case CommandBufferType::SECONDARY:return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    default: {
        assert(true && "not support type");
    }

    }
    return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
}

static void createTextureData(const Device *context, Texture *texture,
                              Format format, const void *data,
                              int num_data_mipmaps, int num_data_layer)
{
    VkImageUsageFlags usage_flags = toImageUsageFlags(texture->format);

    VulkanUtils::createImage(context, texture->type,
                             texture->width, texture->height, texture->depth,
                             texture->num_layers, texture->num_mipmaps, texture->samples,
                             texture->format, texture->tiling,
                             VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                 | VK_IMAGE_USAGE_SAMPLED_BIT | usage_flags,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture->flags,
                             texture->image, texture->imageMemory);

    VkImageLayout source_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (data != nullptr) {
        // prepare for transfer
        VulkanUtils::transitionImageLayout(
            context,
            texture->image,
            texture->format,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            0, texture->num_mipmaps,
            0, texture->num_layers
        );

        VulkanUtils::fillImage(
            context, texture->image,
            texture->width, texture->height, texture->depth,
            texture->num_mipmaps, texture->num_layers, toPixelSize(format), data,
            texture->format, num_data_mipmaps, num_data_layer);

        source_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }

    VulkanUtils::transitionImageLayout(context, texture->image, texture->format,
                                       source_layout,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                       0, texture->num_mipmaps,
                                       0, texture->num_layers);

    // Create image view & sampler
    texture->view = VulkanUtils::createImageView(
        context->LogicDevice(),
        texture->image,
        texture->format,
        toImageAspectFlags(texture->format),
        toImageBaseViewType(texture->type, texture->flags, texture->num_layers),
        0, texture->num_mipmaps,
        0, texture->num_layers
    );

    texture->sampler = VulkanUtils::createSampler(context->LogicDevice(), 0, texture->num_mipmaps);

}

static void selectOptimalSwapChainSettings(Device *context, SwapChain *swapchain)
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

static bool createSwapchainObjects(Device *context,
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
        swapchain->views[i] = VulkanUtils::createImageView(context->LogicDevice(),
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

static void destroySwapchainObjects(Device *context, SwapChain *swapchain)
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

static void updateBindSetLayout(const Device *device, BindSet *bind_set, VkDescriptorSetLayout new_layout)
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

#endif //GAMEENGINE_SRC_BACKEND_VULKAN_AUXILIARY_H
