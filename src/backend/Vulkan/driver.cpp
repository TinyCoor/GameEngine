//
// Created by 12132 on 2021/11/27.
//
#include "../API.h"
#include "driver.h"
#include "VulkanUtils.h"
#include "VulkanContext.h"
#include "Macro.h"
#include "shaderc.h"
#include "VulkanRenderPassBuilder.h"
#include <volk.h>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <limits>
#include <functional>
#include <GLFW/glfw3.h>
#include "platform.h"

namespace render::backend {
namespace shaderc {
static shaderc_shader_kind vulkan_to_shaderc_kind(ShaderType type) {
  switch (type) {
    // Graphics pipeline
  case ShaderType::VERTEX: return shaderc_vertex_shader;
  case ShaderType::TESSELLATION_CONTROL: return shaderc_tess_control_shader;
  case ShaderType::TESSELLATION_EVALUATION: return shaderc_tess_evaluation_shader;
  case ShaderType::GEOMETRY: return shaderc_geometry_shader;
  case ShaderType::FRAGMENT: return shaderc_fragment_shader;

    // Compute pipeline
  case ShaderType::COMPUTE: return shaderc_compute_shader;

#if NV_EXTENSIONS
    // Raytrace pipeline
                case ShaderType::RAY_GENERATION: return shaderc_raygen_shader;
                case ShaderType::INTERSECTION: return shaderc_intersection_shader;
                case ShaderType::ANY_HIT: return shaderc_anyhit_shader;
                case ShaderType::CLOSEST_HIT: return shaderc_closesthit_shader;
                case ShaderType::MISS: return shaderc_miss_shader;
                case ShaderType::CALLABLE: return shaderc_callable_shader;
#endif
  }

  return shaderc_glsl_infer_from_source;
}

static shaderc_include_result *includeResolver(
    void *user_data,
    const char *requested_source,
    int type,
    const char *requesting_source,
    size_t include_depth
) {
  shaderc_include_result *result = new shaderc_include_result();
  result->user_data = user_data;
  result->source_name = nullptr;
  result->source_name_length = 0;
  result->content = nullptr;
  result->content_length = 0;

  std::string target_dir = "";

  switch (type) {
  case shaderc_include_type_standard: {
    // TODO: remove this, not generic
    target_dir = "shaders/";
  }
    break;

  case shaderc_include_type_relative: {
    std::string_view source_path = requesting_source;
    size_t pos = source_path.find_last_of("/\\");

    if (pos != std::string_view::npos)
      target_dir = source_path.substr(0, pos + 1);
  }
    break;
  }

  std::string target_path = target_dir + std::string(requested_source);

  std::ifstream file(target_path, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    std::cerr << "shaderc::include_resolver(): can't load include at \"" << target_path << "\"" << std::endl;
    return result;
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  char *buffer = new char[fileSize];

  file.seekg(0);
  file.read(buffer, fileSize);
  file.close();

  char *path = new char[target_path.size() + 1];
  memcpy(path, target_path.c_str(), target_path.size());
  path[target_path.size()] = '\x0';

  result->source_name = path;
  result->source_name_length = target_path.size() + 1;
  result->content = buffer;
  result->content_length = fileSize;

  return result;
}

static void includeResultReleaser(void *userData, shaderc_include_result *result) {
  delete result->source_name;
  delete result->content;
  delete result;
}
}
}
namespace render::backend::vulkan {
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
          VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16_UINT, VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16_SFLOAT,
          VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R16G16B16_UINT, VK_FORMAT_R16G16B16_SINT, VK_FORMAT_R16G16B16_SFLOAT,
          VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R16G16B16A16_UINT, VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SFLOAT,

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

static VkIndexType ToVkIndexType(IndexSize index_size) {
  static VkIndexType supports[static_cast<unsigned int>(IndexSize::MAX)] = {
      VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32
  };
  return supports[static_cast<uint32_t>(index_size)];
}

static VkPrimitiveTopology toPrimitiveTopology(RenderPrimitiveType type) {
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

static size_t toPixelSize(Format format) {
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

  switch (format)
  {
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
  switch (format)
  {
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

    default:
    {
      std::cerr << "vulkan::fromFormat(): unsupported format " << format << std::endl;
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
  if (samples & VK_SAMPLE_COUNT_64_BIT) { return Multisample::COUNT_64; }
  if (samples & VK_SAMPLE_COUNT_32_BIT) { return Multisample::COUNT_32; }
  if (samples & VK_SAMPLE_COUNT_16_BIT) { return Multisample::COUNT_16; }
  if (samples & VK_SAMPLE_COUNT_8_BIT) { return Multisample::COUNT_8; }
  if (samples & VK_SAMPLE_COUNT_4_BIT) { return Multisample::COUNT_4; }
  if (samples & VK_SAMPLE_COUNT_2_BIT) { return Multisample::COUNT_2; }

  return Multisample::COUNT_1;
}

static VkImageUsageFlags toImageUsageFlags(VkFormat format)
{
  if (format == VK_FORMAT_UNDEFINED)
    return 0;

  switch (format)
  {
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

static void createTextureData(const VulkanContext *context, Texture *texture,
                        Format format, const void *data,
                        int num_data_mipmaps, int num_data_layer) {
  VkImageUsageFlags usage_flags = toImageUsageFlags(texture->format);

  VulkanUtils::createImage(context, texture->type,
                           texture->width, texture->height, texture->depth,
                           texture->num_layers,texture->num_mipmaps, texture->samples,
                           texture->format, texture->tiling,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | usage_flags,
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
      context->Device(),
      texture->image,
      texture->format,
      toImageAspectFlags(texture->format),
      toImageBaseViewType(texture->type, texture->flags, texture->num_layers),
      0, texture->num_mipmaps,
      0, texture->num_layers
  );

  texture->sampler = VulkanUtils::createSampler(context->Device(), 0, texture->num_mipmaps);

}

static void selectOptimalSwapChainSettings(VulkanContext* context, SwapChain* swapchain)
{
  //get surface capability
  auto res =vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->PhysicalDevice(),swapchain->surface,&swapchain->surface_capabilities);
  if ( res !=VK_SUCCESS){
     std::cerr << "get surface capability error: %d"<< res <<std::endl;
  }

  // select best surface format
  uint32_t num_surface_format = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(context->PhysicalDevice(),swapchain->surface,&num_surface_format, nullptr);
  assert(num_surface_format > 0);
  std::vector<VkSurfaceFormatKHR> surface_formats(num_surface_format);
  vkGetPhysicalDeviceSurfaceFormatsKHR(context->PhysicalDevice(),swapchain->surface,&num_surface_format, surface_formats.data());
  // no preferred format
  if(surface_formats.size() ==1 && surface_formats[0].format == VK_FORMAT_UNDEFINED){
    swapchain->surface_format = {VK_FORMAT_B8G8R8A8_UNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

  }else{  // Otherwise, select one of the available formatselse
    swapchain->surface_format = surface_formats[0];
    for (const auto &format : surface_formats)
    {
      if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      {
        swapchain->surface_format = format;
        break;
      }
    }
  }

  //select best present mode
  uint32_t num_present_mode = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(context->PhysicalDevice(),swapchain->surface,&num_present_mode, nullptr);
  assert(num_present_mode >0);
  std::vector<VkPresentModeKHR> present_modes(num_present_mode);
  vkGetPhysicalDeviceSurfacePresentModesKHR(context->PhysicalDevice(),swapchain->surface,&num_present_mode, present_modes.data());
  swapchain->present_mode = VK_PRESENT_MODE_FIFO_KHR;
  for (const auto &presentMode : present_modes)
  {
    // Some drivers currently don't properly support FIFO present mode,
    // so we should prefer IMMEDIATE mode if MAILBOX mode is not available
    if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
      swapchain->present_mode = presentMode;

    if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
    {
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

  if(capabilities.maxImageCount > 0 ){
    swapchain->num_images  = std::min(swapchain->num_images,capabilities.maxImageCount);
  }

}

static bool  createTransientSwapchainObjects(VulkanContext* context, SwapChain* swapchain,uint32_t width,uint32_t height)
{

  const VkSurfaceCapabilitiesKHR &capabilities = swapchain->surface_capabilities;
  VkSwapchainCreateInfoKHR swapChainInfo{};
  swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapChainInfo.surface = swapchain->surface;
  swapChainInfo.minImageCount = swapchain->num_images;
  swapChainInfo.imageFormat = swapchain->surface_format.format; // settings.format.format;
  swapChainInfo.imageColorSpace = swapchain->surface_format.colorSpace;    //settings.format.colorSpace;
  swapChainInfo.imageExtent =swapchain->sizes ;//  swapChainExtent;
  swapChainInfo.imageArrayLayers= 1;
  swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  if(context->GraphicsQueueFamily() != swapchain->present_queue_family ){
    uint32_t queueFamilies[] = {context->GraphicsQueueFamily(),swapchain->present_queue_family};
    swapChainInfo.imageSharingMode =VK_SHARING_MODE_CONCURRENT;
    swapChainInfo.queueFamilyIndexCount = 2;
    swapChainInfo.pQueueFamilyIndices = queueFamilies;

  } else{
    swapChainInfo.imageSharingMode =VK_SHARING_MODE_EXCLUSIVE;
    swapChainInfo.queueFamilyIndexCount = 0;
    swapChainInfo.pQueueFamilyIndices = nullptr;
  }

  swapChainInfo.preTransform = capabilities.currentTransform;
  swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapChainInfo.presentMode =  swapchain->present_mode; // settings.presentMode;
  swapChainInfo.clipped =VK_TRUE;
  swapChainInfo.oldSwapchain = VK_NULL_HANDLE;
  if (vkCreateSwapchainKHR(context->Device(), &swapChainInfo, nullptr, &swapchain->swap_chain) != VK_SUCCESS)
  {
    std::cerr << "vulkan::createTransientSwapChainObjects(): vkCreateSwapchainKHR failed" << std::endl;
    return false;
  }

  vkGetSwapchainImagesKHR(context->Device(),swapchain->swap_chain,&swapchain->num_images, nullptr);
  assert(swapchain->num_images > 0  && swapchain->num_images < render::backend::vulkan::SwapChain::MAX_IMAGES);
  vkGetSwapchainImagesKHR(context->Device(),swapchain->swap_chain,&swapchain->num_images,swapchain->images);

  for (int i = 0; i <swapchain->num_images; ++i) {
    swapchain->views[i] = VulkanUtils::createImageView(context->Device(),
                                                          swapchain->images[i],
                                                          swapchain->surface_format.format,
                                                          VK_IMAGE_ASPECT_COLOR_BIT,
                                                          VK_IMAGE_VIEW_TYPE_2D);
  }

  return true;
}

static void destroyTransientSwapchainObjects(VulkanContext* context, SwapChain* swapchain)
{
  for (size_t i = 0; i < swapchain->num_images ; ++i) {
    vkDestroyImageView(context->Device(),swapchain->views[i], nullptr);
    swapchain->views[i] =VK_NULL_HANDLE;
    swapchain->images[i] =VK_NULL_HANDLE;
  }
  vkDestroySwapchainKHR(context->Device(),swapchain->swap_chain, nullptr);
  swapchain->swap_chain = VK_NULL_HANDLE;
}

VulkanDriver::~VulkanDriver() noexcept {
  if (context != nullptr){
    context->shutdown();
    delete context;
  }
}
VertexBuffer *VulkanDriver::createVertexBuffer(BufferType type,
                                         uint16_t vertex_size,
                                         uint32_t num_vertices,
                                         uint8_t num_attributes,
                                         const VertexAttribute *attributes,
                                         const void *data) {
  assert(type == BufferType::STATIC && "Dynamic are not impl");
  assert(num_vertices != 0 && data != nullptr && "Invalid data");
  assert(vertex_size != 0 && data != nullptr && "Invalid VertexSize");
  assert(num_attributes <= vulkan::VertexBuffer::MAX_ATTRIBUTES && "Vetex Attribute limit to 16");

  VkDeviceSize buffer_size = vertex_size * num_vertices;
  vulkan::VertexBuffer *result = new vulkan::VertexBuffer;
  result->vertex_size = vertex_size;
  result->num_vertices = num_vertices;
  result->num_attributes = num_attributes;

  memcpy(result->attributes, attributes, num_attributes * sizeof (VertexAttribute));

  VulkanUtils::createDeviceLocalBuffer(context, buffer_size,
                                       data,
                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                       result->buffer,
                                       result->memory);

  return result;
}

IndexBuffer *VulkanDriver::createIndexBuffer(BufferType type,
                                       IndexSize index_size,
                                       uint32_t num_indices,
                                       const void *data) {
  assert(type == BufferType::STATIC && "Dynamic are not impl");
  assert(num_indices != 0 && data != nullptr && "Invalid data");
  assert(static_cast<uint32_t>(index_size) != 0 && data != nullptr && "Invalid VertexSize");
  IndexBuffer *indexBuffer = new IndexBuffer();
  indexBuffer->num_indices = num_indices;
  indexBuffer->type = ToVkIndexType(index_size);

  VkDeviceSize buffer_size = vulkan::toIndexSize(index_size) * num_indices;

  VulkanUtils::createDeviceLocalBuffer(context,
                                      buffer_size,
                                       data,
                                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                       indexBuffer->buffer,
                                       indexBuffer->memory);

  return indexBuffer;
}

RenderPrimitive *VulkanDriver::createRenderPrimitive(RenderPrimitiveType type,
                                               const render::backend::VertexBuffer *vertex_buffer,
                                               const render::backend::IndexBuffer *index_buffer) {
  const vulkan::VertexBuffer *v_buffer = static_cast<const vulkan::VertexBuffer *>(vertex_buffer);
  const vulkan::IndexBuffer *i_buffer = static_cast<const vulkan::IndexBuffer *>(index_buffer);

  vulkan::RenderPrimitive *primitive = new vulkan::RenderPrimitive;
  primitive->vertexBuffer = v_buffer;
  primitive->indexBuffer = i_buffer;
  primitive->type = toPrimitiveTopology(type);
  return primitive;
}

Texture *VulkanDriver::createTexture2D(uint32_t width,
                                 uint32_t height,
                                 uint32_t num_mipmaps,
                                 Format format,
                                 Multisample samples ,
                                 const void *data,
                                 uint32_t num_data_mipmaps) {
  vulkan::Texture *texture = new vulkan::Texture();
  texture->width = width;
  texture->height = height;
  texture->depth =1;
  texture->format = toFormat(format);
  texture->num_mipmaps = num_mipmaps;
  texture->num_layers = 1;
  texture->type = VK_IMAGE_TYPE_2D;
  texture->samples = vulkan::toSamples(samples);
  texture->tiling = VK_IMAGE_TILING_OPTIMAL;
  texture->flags = 0;

  createTextureData(context, texture, format, data, num_data_mipmaps, 1);
  return texture;
}

Texture *VulkanDriver::createTexture2DArray(uint32_t width,
                                      uint32_t height,
                                      uint32_t num_mipmaps,
                                      uint32_t num_layers,
                                      Format format,
                                      const void *data,
                                      uint32_t num_data_mipmaps,
                                      uint32_t num_data_layers) {

  vulkan::Texture *texture = new vulkan::Texture();
  texture->type = VK_IMAGE_TYPE_2D;
  texture->width = width;
  texture->height = height;
  texture->depth = 1;
  texture->format = toFormat(format);
  texture->num_mipmaps = num_mipmaps;
  texture->num_layers = num_layers;
  texture->samples = VK_SAMPLE_COUNT_1_BIT;
  texture->tiling = VK_IMAGE_TILING_OPTIMAL;
  texture->flags = 0;

  createTextureData(context, texture, format, data, num_data_mipmaps, num_data_layers);

  return texture;
}

Texture *VulkanDriver::createTexture3D(uint32_t width,
                                 uint32_t height,
                                 uint32_t depth,
                                 uint32_t num_mipmaps,
                                 Format format,
                                 const void *data,
                                 uint32_t num_data_mipmaps) {
  vulkan::Texture *texture = new vulkan::Texture();
  texture->type = VK_IMAGE_TYPE_3D;
  texture->width = width;
  texture->height = height;
  texture->depth = depth;
  texture->format = toFormat(format);
  texture->num_mipmaps = num_mipmaps;
  texture->num_layers = 1;
  texture->samples = VK_SAMPLE_COUNT_1_BIT;
  texture->tiling = VK_IMAGE_TILING_OPTIMAL;
  texture->flags = 0;

  createTextureData(context, texture, format, data, num_data_mipmaps, 1);
  return texture;
}

Texture *VulkanDriver::createTextureCube(uint32_t width,
                                   uint32_t height,
                                   uint32_t num_mipmaps,
                                   Format format,
                                   const void *data,
                                   uint32_t num_data_mipmaps) {
  vulkan::Texture *texture = new vulkan::Texture();
  texture->type = VK_IMAGE_TYPE_2D;
  texture->width = width;
  texture->height = height;
  texture->depth = 1;
  texture->format = toFormat(format);
  texture->num_mipmaps = num_mipmaps;
  texture->num_layers = 6;
  texture->samples = VK_SAMPLE_COUNT_1_BIT;
  texture->tiling = VK_IMAGE_TILING_OPTIMAL;
  texture->flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

  createTextureData(context, texture, format, data, num_data_mipmaps, 1);
  return texture;
}

FrameBuffer *VulkanDriver::createFrameBuffer(uint8_t num_attachments,
                                             const FrameBufferAttachment *attachments) {
  // TODO: check for equal sizes (color + depthstencil)

  vulkan::FrameBuffer *result = new vulkan::FrameBuffer();

  uint32_t width = 0;
  uint32_t height = 0;
  VulkanRenderPassBuilder builder = VulkanRenderPassBuilder(context);

  builder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS);

  // add color attachments
  result->num_attachments = 0;
  for (uint8_t i = 0; i < num_attachments; ++i)
  {
    const FrameBufferAttachment &attachment = attachments[i];
    VkImageView view = VK_NULL_HANDLE;

    if (attachment.type == FrameBufferAttachmentType::COLOR)
    {
      //TODO ERROR IN
      const FrameBufferAttachment::Color &color = attachment.color;
      const vulkan::Texture *color_texture = static_cast<const vulkan::Texture *>(color.texture);
      VkImageAspectFlags flags = vulkan::toImageAspectFlags(color_texture->format);

      view = VulkanUtils::createImageView(
          context->Device(),
          color_texture->image, color_texture->format,
          flags, VK_IMAGE_VIEW_TYPE_2D,
          color.base_mip, color.num_mips,
          color.base_layer, color.num_layers
      );

      width = std::max<int>(1, color_texture->width / (1 << color.base_mip));
      height = std::max<int>(1, color_texture->height / (1 << color.base_mip));

      if (color.resolve_attachment)
      {
        builder.addColorResolveAttachment(color_texture->format);
        builder.addColorResolveAttachmentReference(0, i);
      }
      else
      {
        builder.addColorAttachment(color_texture->format, color_texture->samples);
        builder.addColorAttachmentReference(0, i);
      }
    }
    else if (attachment.type == FrameBufferAttachmentType::DEPTH)
    {
      const FrameBufferAttachment::Depth &depth = attachment.depth;
      const vulkan::Texture *depth_texture = static_cast<const vulkan::Texture *>(depth.texture);
      VkImageAspectFlags flags = vulkan::toImageAspectFlags(depth_texture->format);

      view = VulkanUtils::createImageView(
          context->Device(),
          depth_texture->image, depth_texture->format,
          flags, VK_IMAGE_VIEW_TYPE_2D
      );

      width = depth_texture->width;
      height = depth_texture->height;

      builder.addDepthStencilAttachment(depth_texture->format, depth_texture->samples);
      builder.setDepthStencilAttachmentReference(0, i);
    }
    else if (attachment.type == FrameBufferAttachmentType::SWAP_CHAIN_COLOR)
    {
      const FrameBufferAttachment::SwapChainColor &swap_chain_color = attachment.swap_chain_color;
      const vulkan::SwapChain *swap_chain = static_cast<const vulkan::SwapChain *>(swap_chain_color.swap_chain);
      VkImageAspectFlags flags = vulkan::toImageAspectFlags(swap_chain->surface_format.format);

      view = VulkanUtils::createImageView(
          context->Device(),
          swap_chain->images[swap_chain_color.base_image], swap_chain->surface_format.format,
          flags, VK_IMAGE_VIEW_TYPE_2D
      );

      width = swap_chain->sizes.width;
      height = swap_chain->sizes.height;

      if (swap_chain_color.resolve_attachment)
      {
        builder.addColorResolveAttachment(swap_chain->surface_format.format);
        builder.addColorResolveAttachmentReference(0, i);
      }
      else
      {
        builder.addColorAttachment(swap_chain->surface_format.format, VK_SAMPLE_COUNT_1_BIT);
        builder.addColorAttachmentReference(0, i);
      }
    }

    result->attachments[result->num_attachments++] = view;
  }

  // create dummy renderpass
  result->dummy_render_pass = builder.build(); // TODO: move to render pass cache

  // create framebuffer
  VkFramebufferCreateInfo framebufferInfo = {};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = result->dummy_render_pass;
  framebufferInfo.attachmentCount = result->num_attachments;
  framebufferInfo.pAttachments = result->attachments;
  framebufferInfo.width = width;
  framebufferInfo.height = height;
  framebufferInfo.layers = 1;

  if (vkCreateFramebuffer(context->Device(), &framebufferInfo, nullptr, &result->framebuffer) != VK_SUCCESS)
  {
    // TODO: log error
    delete result;
    result = nullptr;
  }

  return result;
}
UniformBuffer *VulkanDriver::createUniformBuffer(BufferType type, uint32_t size, const void *data) {
  assert(type == BufferType::DYNAMIC && "Only dynamic buffers are implemented at the moment");
  assert(size != 0 && "Invalid size");

  vulkan::UniformBuffer *result = new vulkan::UniformBuffer();
  result->size = size;

  VulkanUtils::createBuffer(
      context,
      size,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      result->buffer,
      result->memory
  );

  if (vkMapMemory(context->Device(), result->memory, 0, size, 0, &result->pointer) != VK_SUCCESS)
  {
    // TODO: log error
    delete result;
    return nullptr;
  }

  if (data != nullptr)
    memcpy(result->pointer, data, static_cast<size_t>(size));

  return result;
}
Shader *VulkanDriver::createShaderFromSource(ShaderType type, uint32_t size, 	const char *data,
                                             const char *path) {
// convert GLSL/HLSL code to SPIR-V bytecode
  shaderc_compiler_t compiler = shaderc_compiler_initialize();
  shaderc_compile_options_t options = shaderc_compile_options_initialize();

  // set compile options
  shaderc_compile_options_set_include_callbacks(options, shaderc::includeResolver, shaderc::includeResultReleaser, nullptr);

  // compile shader
  shaderc_compilation_result_t compilation_result = shaderc_compile_into_spv(
      compiler,
      data, size,
      shaderc_glsl_infer_from_source,
      path,
      "main",
      options
  );

  if (shaderc_result_get_compilation_status(compilation_result) != shaderc_compilation_status_success)
  {
    std::cerr << "VulkanDriver::createShaderFromSource(): can't compile shader at \"" << path << "\"" << std::endl;
    std::cerr << "\t" << shaderc_result_get_error_message(compilation_result);

    shaderc_result_release(compilation_result);
    shaderc_compile_options_release(options);
    shaderc_compiler_release(compiler);

    return nullptr;
  }

  size_t bytecode_size = shaderc_result_get_length(compilation_result);
  const uint32_t *bytecode_data = reinterpret_cast<const uint32_t*>(shaderc_result_get_bytes(compilation_result));

  vulkan::Shader *result = new vulkan::Shader();
  result->type = type;
  result->shaderModule = VulkanUtils::createShaderModule(context->Device(), bytecode_data, bytecode_size);

  shaderc_result_release(compilation_result);
  shaderc_compile_options_release(options);
  shaderc_compiler_release(compiler);

  return result;
}
Shader *VulkanDriver::createShaderFromBytecode(ShaderType type, uint32_t size, const void *data) {
  assert(size != 0 && "Invalid size");
  assert(data != nullptr && "Invalid data");

  vulkan::Shader *result = new vulkan::Shader();
  result->type = type;
  result->shaderModule = VulkanUtils::createShaderModule(context->Device(), reinterpret_cast<const uint32_t *>(data), size);

  return result;
}
void VulkanDriver::destroyVertexBuffer(render::backend::VertexBuffer *vertex_buffer) {
  if (vertex_buffer == nullptr)
    return;


  vulkan::VertexBuffer *vk_vertex_buffer = static_cast<vulkan::VertexBuffer *>(vertex_buffer);

  vkDestroyBuffer(context->Device(), vk_vertex_buffer->buffer, nullptr);
  vkFreeMemory(context->Device(), vk_vertex_buffer->memory, nullptr);

  vk_vertex_buffer->buffer = VK_NULL_HANDLE;
  vk_vertex_buffer->memory = VK_NULL_HANDLE;

  delete vertex_buffer;
  vertex_buffer = nullptr;
}
void VulkanDriver::destroyIndexBuffer(render::backend::IndexBuffer *index_buffer) {
  if (index_buffer == nullptr)
    return;

  vulkan::IndexBuffer *vk_index_buffer = static_cast<vulkan::IndexBuffer *>(index_buffer);

  vkDestroyBuffer(context->Device(), vk_index_buffer->buffer, nullptr);
  vkFreeMemory(context->Device(), vk_index_buffer->memory, nullptr);

  vk_index_buffer->buffer = VK_NULL_HANDLE;
  vk_index_buffer->memory = VK_NULL_HANDLE;

  delete index_buffer;
  index_buffer = nullptr;
}
void VulkanDriver::destroyRenderPrimitive(render::backend::RenderPrimitive *render_primitive) {
  if (render_primitive == nullptr)
    return;

  vulkan::RenderPrimitive *vk_render_primitive = static_cast<vulkan::RenderPrimitive *>(render_primitive);

  vk_render_primitive->vertexBuffer = nullptr;
  vk_render_primitive->indexBuffer = nullptr;

  delete render_primitive;
  render_primitive = nullptr;
}
void VulkanDriver::destroyTexture(render::backend::Texture *texture) {
  if (texture == nullptr)
    return;

  vulkan::Texture *vk_texture = static_cast<vulkan::Texture *>(texture);

  vkDestroyImage(context->Device(), vk_texture->image, nullptr);
  vkFreeMemory(context->Device(), vk_texture->imageMemory, nullptr);

  vk_texture->image = VK_NULL_HANDLE;
  vk_texture->imageMemory = VK_NULL_HANDLE;
  vk_texture->format = VK_FORMAT_UNDEFINED;

  delete texture;
  texture = nullptr;
}
void VulkanDriver::destroyFrameBuffer(render::backend::FrameBuffer *frame_buffer) {
  if (frame_buffer == nullptr)
    return;

  vulkan::FrameBuffer *vk_frame_buffer = static_cast<vulkan::FrameBuffer *>(frame_buffer);

  for (uint8_t i = 0; i < vk_frame_buffer->num_attachments; ++i)
  {
    vkDestroyImageView(context->Device(), vk_frame_buffer->attachments[i], nullptr);
    vk_frame_buffer->attachments[i] = VK_NULL_HANDLE;
  }

  vkDestroyFramebuffer(context->Device(), vk_frame_buffer->framebuffer, nullptr);
  vk_frame_buffer->framebuffer = VK_NULL_HANDLE;

  vkDestroyRenderPass(context->Device(), vk_frame_buffer->dummy_render_pass, nullptr);
  vk_frame_buffer->dummy_render_pass = VK_NULL_HANDLE;

  delete frame_buffer;
  frame_buffer = nullptr;
}
void VulkanDriver::destroyUniformBuffer(render::backend::UniformBuffer *uniform_buffer) {
  if (uniform_buffer == nullptr)
    return;

  vulkan::UniformBuffer *vk_uniform_buffer = static_cast<vulkan::UniformBuffer *>(uniform_buffer);

  vkDestroyBuffer(context->Device(), vk_uniform_buffer->buffer, nullptr);
  vkFreeMemory(context->Device(), vk_uniform_buffer->memory, nullptr);

  vk_uniform_buffer->buffer = VK_NULL_HANDLE;
  vk_uniform_buffer->memory = VK_NULL_HANDLE;

  delete uniform_buffer;
  uniform_buffer = nullptr;
}
void VulkanDriver::destroyShader(render::backend::Shader *shader) {
  if (shader == nullptr)
    return;

  vulkan::Shader *vk_shader = static_cast<vulkan::Shader *>(shader);

  vkDestroyShaderModule(context->Device(), vk_shader->shaderModule, nullptr);
  vk_shader->shaderModule = VK_NULL_HANDLE;

  delete shader;
  shader = nullptr;
}
void VulkanDriver::beginRenderPass(const render::backend::FrameBuffer *frame_buffer) {

}
void VulkanDriver::endRenderPass() {

}
void VulkanDriver::bindUniformBuffer(uint32_t unit, const render::backend::UniformBuffer *uniform_buffer) {

}
void VulkanDriver::bindTexture(uint32_t unit, const render::backend::Texture *texture) {

}
void VulkanDriver::bindShader(const render::backend::Shader *shader) {

}
void VulkanDriver::drawIndexedPrimitive(const render::backend::RenderPrimitive *render_primitive) {

}
void VulkanDriver::drawIndexedPrimitiveInstanced(const render::backend::RenderPrimitive *primitive,
                                           const render::backend::VertexBuffer *instance_buffer,
                                           uint32_t num_instances,
                                           uint32_t offset) {

}
VulkanDriver::VulkanDriver(const char *app_name, const char *engine_name):context(new VulkanContext) {
  context->init(app_name,engine_name);
}

SwapChain *VulkanDriver::createSwapChain(void *native_window,uint32_t width,uint32_t height) {
  assert(native_window != nullptr && "native_window nullptr");
  vulkan::SwapChain* swapchain = new vulkan::SwapChain;

  //create surface
  //  swapchain->surface = vulkan::platform::createSurface(context,native_window);
  //  if(swapchain->surface ==VK_NULL_HANDLE){
  //      std::cerr << "create Platform surface failed" <<std::endl;
  //  }
  VK_CHECK( glfwCreateWindowSurface(context->Instance(),(GLFWwindow*)native_window,
                                    nullptr,&swapchain->surface),"Create Surface failed");


  // get present queue family
  swapchain->present_queue_family = VulkanUtils::fetchPresentQueueFamily(
      context->PhysicalDevice(),
      swapchain->surface,
      context->GraphicsQueueFamily()
  );

  // Get present queue
  vkGetDeviceQueue(context->Device(), swapchain->present_queue_family, 0, &swapchain->present_queue);
  if (swapchain->present_queue == VK_NULL_HANDLE){
    std::cerr <<"Can't get present queue from logical device" << std::endl;
    return nullptr;
  }

  // select swapchain settings
  vulkan::selectOptimalSwapChainSettings(context,swapchain);


  //Create Sync Object
  for (size_t i = 0; i <swapchain->num_images ; ++i) {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateSemaphore(context->Device(), &semaphoreInfo, nullptr, &swapchain->image_available_gpu[i]) != VK_SUCCESS ||
        vkCreateSemaphore(context->Device(), &semaphoreInfo, nullptr, &swapchain->rendering_finished_gpu[i]) != VK_SUCCESS ||
        vkCreateFence(context->Device(), &fenceInfo, nullptr, &swapchain->rendering_finished_cpu[i]) != VK_SUCCESS) {
      std::cerr << "failed to create semaphores!"<<std::endl;
      destroySwapChain(swapchain);
      return nullptr;
    }
  }
  createTransientSwapchainObjects(context,swapchain,width,height);

  return swapchain;
}

void VulkanDriver::destroySwapChain(render::backend::SwapChain *swapchain) {
  if (swapchain == nullptr)
    return;

  auto* vk_swap_chain = static_cast<vulkan::SwapChain*>(swapchain);

  //Destroy Sync Object
  for (size_t i = 0; i <vk_swap_chain->num_images ; ++i) {
      vkDestroySemaphore(context->Device(), vk_swap_chain->image_available_gpu[i], nullptr);
      vk_swap_chain->image_available_gpu[i] = VK_NULL_HANDLE;
      vkDestroySemaphore(context->Device(),vk_swap_chain->rendering_finished_gpu[i], nullptr );
      vk_swap_chain->rendering_finished_gpu[i] = VK_NULL_HANDLE;
      vkDestroyFence(context->Device(),vk_swap_chain->rendering_finished_cpu[i], nullptr) ;
      vk_swap_chain->rendering_finished_cpu[i] =VK_NULL_HANDLE;
  }

  vk_swap_chain->present_queue_family= 0xFFFF;
  vk_swap_chain->present_queue= VK_NULL_HANDLE;
  vk_swap_chain->present_mode =VK_PRESENT_MODE_FIFO_KHR;
  vk_swap_chain->sizes = {0,0};

  vk_swap_chain->num_images = 0;
  vk_swap_chain->current_image = 0;

  destroyTransientSwapchainObjects(context,vk_swap_chain);

  // destroy Surface
  vulkan::platform::destroySurface(context,&vk_swap_chain->surface);
  vk_swap_chain->surface =VK_NULL_HANDLE;
  delete swapchain;
  swapchain= nullptr;
}



bool VulkanDriver::acquire(render::backend::SwapChain *swap_chain,uint32_t* image_index) {
  vulkan::SwapChain *vk_swap_chain = static_cast<vulkan::SwapChain *>(swap_chain);
  uint32_t current_image = vk_swap_chain->current_image;

  vkWaitForFences(
      context->Device(),
      1, &vk_swap_chain->rendering_finished_cpu[current_image],
      VK_TRUE, std::numeric_limits<uint64_t>::max()
  );

  VkResult result = vkAcquireNextImageKHR(
      context->Device(),
      vk_swap_chain->swap_chain,
      std::numeric_limits<uint64_t>::max(),
      vk_swap_chain->image_available_gpu[current_image],
      VK_NULL_HANDLE,
      image_index
  );

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
    return false;

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
  {
    // TODO: log fatal
    // runtime_error("Can't acquire swap chain image");
    return false;
  }

  return true;
}

bool VulkanDriver::present(render::backend::SwapChain  *swap_chain) {
  vulkan::SwapChain *vk_swap_chain = static_cast<vulkan::SwapChain *>(swap_chain);
  uint32_t current_image = vk_swap_chain->current_image;

  VkPresentInfoKHR info = {};
  info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  info.waitSemaphoreCount = 1;
  info.pWaitSemaphores = &vk_swap_chain->rendering_finished_gpu[current_image]; // TODO: check if we need to wait for this
  info.swapchainCount = 1;
  info.pSwapchains = &vk_swap_chain->swap_chain;
  info.pImageIndices = &current_image;

  VulkanUtils::transitionImageLayout(
      context,
      vk_swap_chain->images[current_image],
      vk_swap_chain->surface_format.format,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  );

  VkResult result = vkQueuePresentKHR(vk_swap_chain->present_queue, &info);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    return false;

  if (result != VK_SUCCESS)
  {
    // TODO: log fatal
    // runtime_error("Can't present swap chain image");
    return false;
  }

  vk_swap_chain->current_image++;
  vk_swap_chain->current_image %= vk_swap_chain->num_images;

  return true;
}

bool VulkanDriver::resize(render::backend::SwapChain *swapchain, uint32_t width, uint32_t height) {
  return false;
}


void VulkanDriver::wait() {
  assert(context != nullptr);
  context->wait();
}

void *VulkanDriver::map(render::backend::UniformBuffer *uniform_buffer) {
  assert(uniform_buffer != nullptr && "Invalid uniform buffer");
  // TODO: check DYNAMIC buffer type

  vulkan::UniformBuffer *vk_uniform_buffer = static_cast<vulkan::UniformBuffer *>(uniform_buffer);

  // NOTE: here we should call vkMapMemory but since it was called during UBO creation, we do nothing here.
  //       It's important to do a proper stress test to see if we can map all previously created UBOs.
  return vk_uniform_buffer->pointer;
}

void VulkanDriver::unmap(render::backend::UniformBuffer *uniform_buffer) {

}

void VulkanDriver::generateTexture2DMipmaps(render::backend::Texture *texture) {
  assert(texture != nullptr && "Invalid texture");

  vulkan::Texture *vk_texture = static_cast<vulkan::Texture *>(texture);

  // prepare for transfer
  VulkanUtils::transitionImageLayout(
      context,
      vk_texture->image,
      vk_texture->format,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      0,
      vk_texture->num_mipmaps
  );

  VulkanUtils::generateImage2DMipMaps(
      context,
      vk_texture->image,
      vk_texture->format,
      vk_texture->width,
      vk_texture->height,
      vk_texture->num_mipmaps,
      vk_texture->format,
      VK_FILTER_LINEAR
  );

  // prepare for shader access
  VulkanUtils::transitionImageLayout(
      context,
      vk_texture->image,
      vk_texture->format,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      0,
      vk_texture->num_mipmaps
  );
}
Multisample VulkanDriver::getMaxSampleCount()
{
  assert(context != nullptr && "Invalid context");

  VkSampleCountFlagBits samples = context->getMaxSampleCount();
  return vulkan::fromSamples(samples);
}

Format VulkanDriver::getOptimalDepthFormat()
{
  assert(context != nullptr && "Invalid context");

  VkFormat format = VulkanUtils::selectOptimalImageFormat(context->PhysicalDevice());
  return vulkan::fromFormat(format);
}

VkSampleCountFlagBits VulkanDriver::toMultisample(Multisample samples)
{
  return toSamples(samples);
}

Multisample VulkanDriver::fromMultisample(VkSampleCountFlagBits samples)
{
  return vulkan::fromSamples(samples);
}

VkFormat VulkanDriver::toFormat(Format format)
{
  return vulkan::toVkFormat(format);
}

Format VulkanDriver::fromFormat(VkFormat format)
{
  return vulkan::fromFormat(format);
}
}
