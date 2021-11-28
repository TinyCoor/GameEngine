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

///todo array support
static VkFormat toVKFormat(render::backend::Format format) {
#define CASE_CONCAT(prefix, name) case name: return prefix##_##name;
  switch (format) {
  CASE_CONCAT(VK_FORMAT, UNDEFINED)
  CASE_CONCAT(VK_FORMAT, R8_UNORM)
  CASE_CONCAT(VK_FORMAT, R8_SNORM)
  CASE_CONCAT(VK_FORMAT, R8_UINT)
  CASE_CONCAT(VK_FORMAT, R8_SINT)
  CASE_CONCAT(VK_FORMAT, R8G8_UNORM)
  CASE_CONCAT(VK_FORMAT, R8G8_SNORM)
  CASE_CONCAT(VK_FORMAT, R8G8_UINT)
  CASE_CONCAT(VK_FORMAT, R8G8_SINT)
  CASE_CONCAT(VK_FORMAT, R8G8B8_UNORM)
  CASE_CONCAT(VK_FORMAT, R8G8B8_SNORM)
  CASE_CONCAT(VK_FORMAT, R8G8B8_UINT)
  CASE_CONCAT(VK_FORMAT, R8G8B8_SINT)
  CASE_CONCAT(VK_FORMAT, R8G8B8A8_UNORM)
  CASE_CONCAT(VK_FORMAT, R8G8B8A8_SNORM)
  CASE_CONCAT(VK_FORMAT, R8G8B8A8_UINT)
  CASE_CONCAT(VK_FORMAT, R8G8B8A8_SINT)

    // 16 bit
  CASE_CONCAT(VK_FORMAT, R16_UNORM)
  CASE_CONCAT(VK_FORMAT, R16_SNORM)
  CASE_CONCAT(VK_FORMAT, R16_UINT)
  CASE_CONCAT(VK_FORMAT, R16_SINT)
  CASE_CONCAT(VK_FORMAT, R16_SFLOAT)
  CASE_CONCAT(VK_FORMAT, R16G16_UNORM)
  CASE_CONCAT(VK_FORMAT, R16G16_SNORM)
  CASE_CONCAT(VK_FORMAT, R16G16_UINT)
  CASE_CONCAT(VK_FORMAT, R16G16_SINT)
  CASE_CONCAT(VK_FORMAT, R16G16_SFLOAT)
  CASE_CONCAT(VK_FORMAT, R16G16B16_UNORM)
  CASE_CONCAT(VK_FORMAT, R16G16B16_SNORM)
  CASE_CONCAT(VK_FORMAT, R16G16B16_UINT)
  CASE_CONCAT(VK_FORMAT, R16G16B16_SINT)
  CASE_CONCAT(VK_FORMAT, R16G16B16_SFLOAT)
  CASE_CONCAT(VK_FORMAT, R16G16B16A16_UNORM)
  CASE_CONCAT(VK_FORMAT, R16G16B16A16_SNORM)
  CASE_CONCAT(VK_FORMAT, R16G16B16A16_UINT)
  CASE_CONCAT(VK_FORMAT, R16G16B16A16_SINT)
  CASE_CONCAT(VK_FORMAT, R16G16B16A16_SFLOAT)

    //32 bits
  CASE_CONCAT(VK_FORMAT, R32_UINT)
  CASE_CONCAT(VK_FORMAT, R32_SINT)
  CASE_CONCAT(VK_FORMAT, R32_SFLOAT)
  CASE_CONCAT(VK_FORMAT, R32G32_UINT)
  CASE_CONCAT(VK_FORMAT, R32G32_SINT)
  CASE_CONCAT(VK_FORMAT, R32G32_SFLOAT)
  CASE_CONCAT(VK_FORMAT, R32G32B32_UINT)
  CASE_CONCAT(VK_FORMAT, R32G32B32_SINT)
  CASE_CONCAT(VK_FORMAT, R32G32B32_SFLOAT)
  CASE_CONCAT(VK_FORMAT, R32G32B32A32_UINT)
  CASE_CONCAT(VK_FORMAT, R32G32B32A32_SINT)
  CASE_CONCAT(VK_FORMAT, R32G32B32A32_SFLOAT)
    //depth
  CASE_CONCAT(VK_FORMAT, D16_UNORM)
  CASE_CONCAT(VK_FORMAT, D16_UNORM_S8_UINT)
  CASE_CONCAT(VK_FORMAT, D24_UNORM_S8_UINT)
  CASE_CONCAT(VK_FORMAT, D32_SFLOAT)
  CASE_CONCAT(VK_FORMAT, D32_SFLOAT_S8_UINT)

  }
#undef CASE_CONCAT

  return VK_FORMAT_UNDEFINED;
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
#define CASE_FORMAT_SIZE(format, count, type) case format : return sizeof(type) * count;
  switch (format) {
  CASE_FORMAT_SIZE(R8_UNORM, 1, uint8_t)
  CASE_FORMAT_SIZE(R8_SNORM, 1, uint8_t)
  CASE_FORMAT_SIZE(R8_UINT, 1, uint8_t)
  CASE_FORMAT_SIZE(R8_SINT, 1, uint8_t)

  CASE_FORMAT_SIZE(R8G8_UNORM, 2, uint8_t)
  CASE_FORMAT_SIZE(R8G8_SNORM, 2, uint8_t)
  CASE_FORMAT_SIZE(R8G8_UINT, 2, uint8_t)
  CASE_FORMAT_SIZE(R8G8_SINT, 2, uint8_t)

  CASE_FORMAT_SIZE(R8G8B8_UNORM, 3, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8_SNORM, 3, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8_UINT, 3, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8_SINT, 3, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8A8_UNORM, 4, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8A8_SNORM, 4, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8A8_UINT, 4, uint8_t)
  CASE_FORMAT_SIZE(R8G8B8A8_SINT, 4, uint8_t)

  CASE_FORMAT_SIZE(R16_UNORM, 1, uint16_t)
  CASE_FORMAT_SIZE(R16_SNORM, 1, uint16_t)
  CASE_FORMAT_SIZE(R16_UINT, 1, uint16_t)
  CASE_FORMAT_SIZE(R16_SINT, 1, uint16_t)
  CASE_FORMAT_SIZE(R16_SFLOAT, 1, uint16_t)

  CASE_FORMAT_SIZE(R16G16_UNORM, 2, uint16_t)
  CASE_FORMAT_SIZE(R16G16_SNORM, 2, uint16_t)
  CASE_FORMAT_SIZE(R16G16_UINT, 2, uint16_t)
  CASE_FORMAT_SIZE(R16G16_SINT, 2, uint16_t)
  CASE_FORMAT_SIZE(R16G16_SFLOAT, 2, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16_UNORM, 3, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16_SNORM, 3, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16_UINT, 3, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16_SINT, 3, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16_SFLOAT, 3, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16A16_UNORM, 4, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16A16_SNORM, 4, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16A16_UINT, 4, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16A16_SINT, 4, uint16_t)
  CASE_FORMAT_SIZE(R16G16B16A16_SFLOAT, 4, uint16_t)

  CASE_FORMAT_SIZE(R32_UINT, 1, uint32_t)
  CASE_FORMAT_SIZE(R32_SINT, 1, uint32_t)
  CASE_FORMAT_SIZE(R32_SFLOAT, 1, uint32_t)
  CASE_FORMAT_SIZE(R32G32_UINT, 2, uint32_t)
  CASE_FORMAT_SIZE(R32G32_SINT, 2, uint32_t)
  CASE_FORMAT_SIZE(R32G32_SFLOAT, 2, uint32_t)
  CASE_FORMAT_SIZE(R32G32B32_UINT, 3, uint32_t)
  CASE_FORMAT_SIZE(R32G32B32_SINT, 3, uint32_t)
  CASE_FORMAT_SIZE(R32G32B32_SFLOAT, 3, uint32_t)
  CASE_FORMAT_SIZE(R32G32B32A32_UINT, 4, uint32_t)
  CASE_FORMAT_SIZE(R32G32B32A32_SINT, 4, uint32_t)
  CASE_FORMAT_SIZE(R32G32B32A32_SFLOAT, 4, uint32_t)

    // depth formats
  CASE_FORMAT_SIZE(D16_UNORM, 2, uint8_t)
  CASE_FORMAT_SIZE(D16_UNORM_S8_UINT, 2, uint8_t)
  CASE_FORMAT_SIZE(D24_UNORM_S8_UINT, 3, uint8_t)
  CASE_FORMAT_SIZE(D32_SFLOAT, 4, uint8_t)
  CASE_FORMAT_SIZE(D32_SFLOAT_S8_UINT, 4, uint8_t)
  }
#undef CASE_FORMAT_SIZE
  return 0;
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
  VulkanUtils::createImage(context, texture->type,
                           texture->width, texture->height, texture->depth,
                           texture->num_layers,texture->num_mipmaps, texture->samples,
                           texture->format, texture->tiling,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                               | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
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
  //get surface capablity
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->PhysicalDevice(),swapchain->surface,&swapchain->surface_capabilities);

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
}

static void createTransientSwapchainObjects(VulkanContext* context, SwapChain* swapchain)
{
  //todo
}

static void destroyTransientSwapchainObjects(VulkanContext* context, SwapChain* swapchain)
{
  //todo
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
                                 const void *data,
                                 uint32_t num_data_mipmaps) {
  vulkan::Texture *texture = new vulkan::Texture();
  texture->width = width;
  texture->height = height;
  texture->depth =1;
  texture->format = toVKFormat(format);
  texture->num_mipmaps = num_mipmaps;
  texture->num_layers = 1;
  texture->type = VK_IMAGE_TYPE_2D;
  texture->samples = VK_SAMPLE_COUNT_1_BIT;
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
  texture->format = toVKFormat(format);
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
  texture->format = toVKFormat(format);
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
  texture->format = toVKFormat(format);
  texture->num_mipmaps = num_mipmaps;
  texture->num_layers = 6;
  texture->samples = VK_SAMPLE_COUNT_1_BIT;
  texture->tiling = VK_IMAGE_TILING_OPTIMAL;
  texture->flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

  createTextureData(context, texture, format, data, num_data_mipmaps, 1);
  return texture;
}

FrameBuffer *VulkanDriver::createFrameBuffer(uint8_t num_color_attachments,
                                       const render::backend::FrameBufferColorAttachment *color_attachments,
                                       const render::backend::FrameBufferDepthStencilAttachment *depthstencil_attachment) {
  assert((depthstencil_attachment != nullptr && num_color_attachments == 0) || (num_color_attachments != 0) && "Invalid attachments");

  // TODO: check for equal sizes (color + depthstencil)

  vulkan::FrameBuffer *result = new vulkan::FrameBuffer();

  VkImageView attachments[vulkan::FrameBuffer::MAX_COLOR_ATTACHMENTS + 1];
  uint8_t num_attachments = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  VulkanRenderPassBuilder builder = VulkanRenderPassBuilder(context);

  builder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS);

  // add color attachments
  result->num_color_attachments = num_color_attachments;
  for (uint8_t i = 0; i < num_color_attachments; ++i)
  {
    backend::FrameBufferColorAttachment attachment = color_attachments[i];
    const vulkan::Texture *texture = static_cast<const vulkan::Texture *>(attachment.texture);

    VkImageView view = VulkanUtils::createImageView(
        context->Device(),
        texture->image, texture->format,
        VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D,
        attachment.base_mip, attachment.num_mips,
        attachment.base_layer, attachment.num_layers
    );

    result->color_attachments[i].view = view;
    attachments[num_attachments++] = view;

    width = std::max<int>(1, texture->width / (1 << attachment.base_mip));
    height = std::max<int>(1, texture->height / (1 << attachment.base_mip));

    builder.addColorAttachment(texture->format, texture->samples);
    builder.addColorAttachmentReference(0, i);
  }

  // add depthstencil attachment
  if (depthstencil_attachment != nullptr)
  {
    const vulkan::Texture *texture = static_cast<const vulkan::Texture *>(depthstencil_attachment->texture);

    VkImageAspectFlags flags = vulkan::toImageAspectFlags(texture->format);
    assert((flags & VK_IMAGE_ASPECT_DEPTH_BIT) && "Invalid depthstencil attachment format");

    VkImageView view = VulkanUtils::createImageView(
        context->Device(),
        texture->image, texture->format,
        flags, VK_IMAGE_VIEW_TYPE_2D
    );

    result->depthstencil_attachment.view = view;
    attachments[num_attachments++] = view;

    builder.addDepthStencilAttachment(texture->format, texture->samples);
    builder.setDepthStencilAttachment(0, num_color_attachments);
  }

  // create dummy renderpass
  result->dummy_render_pass = builder.build(); // TODO: move to render pass cache

  // create framebuffer
  VkFramebufferCreateInfo framebufferInfo = {};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = result->dummy_render_pass;
  framebufferInfo.attachmentCount = num_attachments;
  framebufferInfo.pAttachments = attachments;
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

  for (uint8_t i = 0; i < vk_frame_buffer->num_color_attachments; ++i)
  {
    vkDestroyImageView(context->Device(), vk_frame_buffer->color_attachments[i].view, nullptr);
    vk_frame_buffer->color_attachments[i].view = VK_NULL_HANDLE;
  }

  vkDestroyImageView(context->Device(), vk_frame_buffer->depthstencil_attachment.view, nullptr);
  vk_frame_buffer->depthstencil_attachment.view = VK_NULL_HANDLE;

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

SwapChain *VulkanDriver::createSwapChain(void *native_window) {
  assert(native_window != nullptr && "native_window nullptr");
  vulkan::SwapChain* swapchain = new vulkan::SwapChain;

  //swapchain->surface = vulkan::platform::createSuface(native_window);

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

  createTransientSwapchainObjects(context,swapchain);

  //Create Sync Object
  for (size_t i = 0; i <swapchain->num_images ; ++i) {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateSemaphore(context->Device(), &semaphoreInfo, nullptr, &swapchain->image_available_gpu[i]) != VK_SUCCESS ||
        vkCreateSemaphore(context->Device(), &semaphoreInfo, nullptr, &swapchain->render_finished_gpu[i]) != VK_SUCCESS ||
        vkCreateFence(context->Device(), &fenceInfo, nullptr, &swapchain->rendering_finished_cpu[i]) != VK_SUCCESS) {
      std::cerr << "failed to create semaphores!"<<std::endl;
      destroySwapChain(swapchain);
      return nullptr;
    }
  }

  return swapchain;
}

void VulkanDriver::destroySwapChain(render::backend::SwapChain *swapchain) {
  if (swapchain == nullptr)
    return;

  auto* vk_swap_chain = static_cast<vulkan::SwapChain*>(swapchain);

  //todo free vk resource

  delete swapchain;
  swapchain= nullptr;
}



bool VulkanDriver::acquire(render::backend::SwapChain *swapchain) {
  return false;
}

bool VulkanDriver::present(render::backend::SwapChain  *swapchain) {
  return false;
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

}
