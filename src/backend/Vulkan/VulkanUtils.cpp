//
// Created by y123456 on 2021/10/13.
//

#include "VulkanUtils.h"
#include "Device.h"
#include "Macro.h"
#include <stdexcept>
#include <fstream>
#include <cassert>
#include <iostream>
#include <cstring>

namespace render::backend::vulkan {
uint32_t VulkanUtils::fetchGraphicsQueueFamily(
    VkPhysicalDevice physicalDevice
) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    const auto &queueFamily = queueFamilies[i];
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      return i;
  }

  return 0xFFFF;
}

/*
 */
uint32_t VulkanUtils::fetchPresentQueueFamily(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t graphicsQueueFamily
) {
  VkBool32 presentSupported = VK_FALSE;

  if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, graphicsQueueFamily, surface, &presentSupported)
      != VK_SUCCESS)
    throw std::runtime_error("Can't check surface present support");

  if (presentSupported)
    return graphicsQueueFamily;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupported) != VK_SUCCESS)
      throw std::runtime_error("Can't check surface present support");

    if (presentSupported)
      return i;
  }

  return 0xFFFF;
}

QueueFamilyIndices VulkanUtils::fetchFamilyIndices(VkPhysicalDevice &physicalDevice, VkSurfaceKHR &surface) {
  uint32_t queueCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueFamilies.data());
  QueueFamilyIndices indices{};
  for (int i = 0; i < queueFamilies.size(); ++i) {

    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = std::make_optional(i);
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
    if (queueFamilies[i].queueCount > 0 && presentSupport) {
      indices.presentFamily = std::make_optional(i);
    }
    if (indices.isComplete()) {
      break;
    }
  }
  return indices;
}

void VulkanUtils::createBuffer(const Device *context,
                               VkDeviceSize size,
                               VkBufferUsageFlags usageFlags,
                               VkMemoryPropertyFlags memoryFlags,
                               VkBuffer &buffer,
                               VkDeviceMemory &memory) {
  //Create Vertex Buffer
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usageFlags;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VK_CHECK(vkCreateBuffer(context->LogicDevice(), &bufferInfo, nullptr, &buffer),
           "failed to create vertex buffer!");

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(context->LogicDevice(), buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(context->PhysicalDevice(), memRequirements.memoryTypeBits, memoryFlags);
  VK_CHECK(vkAllocateMemory(context->LogicDevice(), &allocInfo, nullptr, &memory),
           "failed to allocate vertex buffer memory!");
  VK_CHECK(vkBindBufferMemory(context->LogicDevice(), buffer, memory, 0), "Bind Buffer VertexBuffer Failed");

}

uint32_t VulkanUtils::findMemoryType(const VkPhysicalDevice physicalDevice,
                                     uint32_t typeFilter,
                                     VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    uint32_t memoryPropertiy = memProperties.memoryTypes[i].propertyFlags;
    if (typeFilter & (1 << i) && (memoryPropertiy & properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanUtils::copyBuffer(const Device *context,
                             VkBuffer srcBuffer,
                             VkBuffer dstBuffer,
                             VkDeviceSize size) {

  auto commandBuffer = beginSingleTimeCommands(context);

  VkBufferCopy copyRegion = {};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(context, commandBuffer);
}

std::vector<char> VulkanUtils::readFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("open file failed: " + filename);
  }

  size_t file_size = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(file_size);
  file.seekg(0);
  file.read(buffer.data(), (long long) file_size);
  file.close();
  return buffer;
}

VkCommandBuffer VulkanUtils::beginSingleTimeCommands(const Device *context) {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = context->CommandPool();
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(context->LogicDevice(), &allocInfo, &commandBuffer);
  assert(commandBuffer != VK_NULL_HANDLE);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

void VulkanUtils::createImage2D(const Device *context,
                                uint32_t width, uint32_t height,
                                uint32_t mipLevel,
                                VkSampleCountFlagBits numberSample,
                                VkFormat format, VkImageTiling tiling,
                                VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                                VkImage &image, VkDeviceMemory &memory) {
  //Create  Buffer
  createImage(context, VK_IMAGE_TYPE_2D,
              width, height, 1, 1, mipLevel, numberSample,
              format, tiling, usage, properties, 0,
              image, memory);
}

void VulkanUtils::createCubeImage(const Device *context, uint32_t width, uint32_t height,
                                  uint32_t mipLevel, VkSampleCountFlagBits numberSample, VkFormat format,
                                  VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                                  VkImage &image, VkDeviceMemory &memory) {
  //Create  Buffer
  createImage(context, VK_IMAGE_TYPE_2D,
              width, height, 1, 6,
              mipLevel, numberSample, format,
              tiling, usage, properties,
              VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
              image, memory
  );
}

VkImageView VulkanUtils::createImageCubeView(VkDevice device,
                                             VkImage image,
                                             VkFormat format,
                                             VkImageAspectFlags aspectFlags,
                                             uint32_t baseMipLayer,
                                             uint32_t numMipLevel) {
  VkImageView textureImageView{};
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = baseMipLayer;
  viewInfo.subresourceRange.levelCount = numMipLevel;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 6;
  VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &textureImageView), "failed to create texture image view!");
  return textureImageView;
}

VkImageView VulkanUtils::createImageView(VkDevice device,
                                         VkImage image,
                                         VkFormat format,
                                         VkImageAspectFlags aspectFlags,
                                         VkImageViewType viewType,
                                         uint32_t baseMipLevel,
                                         uint32_t numMipLevel,
                                         uint32_t baseLayer,
                                         uint32_t numLayers) {
  VkImageView textureImageView{VK_NULL_HANDLE};
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = viewType;
  viewInfo.format = format;
  viewInfo.components =
      {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = baseMipLevel;
  viewInfo.subresourceRange.levelCount = numMipLevel;
  viewInfo.subresourceRange.baseArrayLayer = baseLayer;
  viewInfo.subresourceRange.layerCount = numLayers;

  VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &textureImageView), "failed to create texture image view!");
  return textureImageView;
}

void VulkanUtils::transitionImageLayout(const Device *context,
                                        VkImage image,
                                        VkFormat format,
                                        VkImageLayout oldLayout,
                                        VkImageLayout newLayout,
                                        uint32_t baseMipLevel,
                                        uint32_t numMipLevels,
                                        uint32_t baseLayer,
                                        uint32_t numLayers) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;

  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = baseMipLevel;
  barrier.subresourceRange.levelCount = numMipLevels;
  barrier.subresourceRange.baseArrayLayer = baseLayer;
  barrier.subresourceRange.layerCount = numLayers;

  if (isDepthFormat(format))
  {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (hasStencilComponent(format))
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
  }

  VkPipelineStageFlags srcStage;
  VkPipelineStageFlags dstStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
  {
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
  {
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;

    srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  }
  else
    throw std::runtime_error("Unsupported layout transition");

  vkCmdPipelineBarrier(
      commandBuffer,
      srcStage, dstStage,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier
  );

  endSingleTimeCommands(context, commandBuffer);
}

void VulkanUtils::copyBufferToImage(const Device *context, VkBuffer srcBuffer, VkImage dstBuffer, uint32_t width,
                                    uint32_t height) {
  auto commandBuffer = beginSingleTimeCommands(context);

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};
  vkCmdCopyBufferToImage(
      commandBuffer,
      srcBuffer,
      dstBuffer,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &region
  );
  endSingleTimeCommands(context, commandBuffer);
}

void VulkanUtils::copyBufferToImage(const Device *context,
                                    VkBuffer srcBuffer,
                                    VkImage dstBuffer,
                                    uint32_t width,
                                    uint32_t height,
                                    uint32_t depth,
                                    uint32_t mipLevel,
                                    uint32_t layer,
                                    VkDeviceSize bufferOffset) {
  auto commandBuffer = beginSingleTimeCommands(context);

  VkBufferImageCopy region{};
  region.bufferOffset = bufferOffset;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = mipLevel;
  region.imageSubresource.baseArrayLayer = layer;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, depth};
  vkCmdCopyBufferToImage(
      commandBuffer,
      srcBuffer,
      dstBuffer,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &region
  );
  endSingleTimeCommands(context, commandBuffer);
}

void VulkanUtils::createImage(
    const Device *context,
    VkImageType imageType,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t arrayLayers,
    uint32_t mipLevel,
    VkSampleCountFlagBits numberSample,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImageCreateFlags flags,
    VkImage &image,
    VkDeviceMemory &memory
) {
  //Create  Buffer
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = imageType;
  imageInfo.extent.width = static_cast<uint32_t>(width);
  imageInfo.extent.height = static_cast<uint32_t>(height);
  imageInfo.extent.depth = depth;
  imageInfo.mipLevels = mipLevel;
  imageInfo.arrayLayers = arrayLayers;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = numberSample;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.flags = flags;

  VK_CHECK(vkCreateImage(context->LogicDevice(), &imageInfo, nullptr, &image),
           "failed to create textures!");

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(context->LogicDevice(), image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(context->PhysicalDevice(), memRequirements.memoryTypeBits, properties);
  VK_CHECK(vkAllocateMemory(context->LogicDevice(), &allocInfo, nullptr, &memory),
           "failed to allocate vertex buffer memory!");
  VK_CHECK(vkBindImageMemory(context->LogicDevice(), image, memory, 0), "Bind Buffer VertexBuffer Failed");
}

void VulkanUtils::fillImage(const Device *context, VkImage &image,
                            uint32_t width, uint32_t height, uint32_t depth,
                            uint32_t mipLevel, uint32_t arrayLayers, uint32_t pixel_size,
                            const void *data, VkFormat format, uint32_t dataMipLevel, uint32_t dataLayers
) {

  VkDeviceSize buffer_size = 0;
  uint32_t mip_width = width;
  uint32_t mip_height = height;
  uint32_t mip_depth = depth;
  for (int i = 0; i < dataMipLevel; ++i) {
    buffer_size += mip_width * mip_height * pixel_size;
    mip_width = std::max(mip_width / 2, 1u);
    mip_height = std::max(mip_height / 2, 1u);
    mip_depth = std::max(mip_depth / 2, 1u);
  }

  /// copy pixel
  VkBuffer stagingBuffer{};
  VkDeviceMemory stagingBufferMemory{};
  VulkanUtils::createBuffer(context, buffer_size * dataLayers,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            stagingBuffer,
                            stagingBufferMemory);

  //
  void *staging_data = nullptr;
  vkMapMemory(context->LogicDevice(), stagingBufferMemory, 0, buffer_size * dataLayers, 0, &staging_data);
  memcpy(staging_data, data, static_cast<size_t>(buffer_size * dataLayers));
  vkUnmapMemory(context->LogicDevice(), stagingBufferMemory);

  // copy to gpu

  VkDeviceSize offset = 0;
  auto commandBuffer = VulkanUtils::beginSingleTimeCommands(context);
  for (int i = 0; i < dataLayers; ++i) {
    mip_width = width;
    mip_height = height;
    mip_depth = depth;
    for (int j = 0; j < dataMipLevel; ++j) {
      VkBufferImageCopy region{};
      region.bufferOffset = offset;
      region.bufferRowLength = 0;
      region.bufferImageHeight = 0;

      region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      region.imageSubresource.mipLevel = j;
      region.imageSubresource.baseArrayLayer = i;
      region.imageSubresource.layerCount = 1;

      region.imageOffset = {0, 0, 0};
      region.imageExtent = {width, height, 1};
      vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             1, &region
      );
      offset += mip_width * mip_height * mip_depth * pixel_size;
      mip_width /= 2;
      mip_height /= 2;
    }
  }

  VulkanUtils::endSingleTimeCommands(context, commandBuffer);
  //clean up
  vkDestroyBuffer(context->LogicDevice(), stagingBuffer, nullptr);
  vkFreeMemory(context->LogicDevice(), stagingBufferMemory, nullptr);
}

VkSampler VulkanUtils::createSampler(VkDevice device,
                                     int minMipLevel,
                                     int maxMipLevel) {
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.maxAnisotropy = 1;//other requires GPU Feature
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_TRUE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = static_cast<float > (minMipLevel);
  samplerInfo.maxLod = static_cast<float>(maxMipLevel);
  VkSampler textureSampler{VK_NULL_HANDLE};
  VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler), "failed to create texture sampler!");

  return textureSampler;
}

bool VulkanUtils::hasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void
VulkanUtils::generateImage2DMipMaps(const Device *context,
                                    VkImage image, VkFormat imageFormat, uint32_t width, uint32_t height,
                                    uint32_t mipLevel, VkFormat format,
                                    VkFilter filter) {
  if (mipLevel == 1) {
    return;
  }
  VkFormatProperties formatProperties;
  vkGetPhysicalDeviceFormatProperties(context->PhysicalDevice(), format, &formatProperties);
  bool supportsLinearFiltering =
      (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) != 0;
  bool supportCubicFiltering =
      (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT) != 0;

  if ((filter == VK_FILTER_LINEAR) && !supportsLinearFiltering) {
    throw std::runtime_error("texture image format does not support linear blitting!");
  }
  if ((filter == VK_FILTER_CUBIC_EXT) && !supportCubicFiltering) {
    throw std::runtime_error("texture image format does not support cubic blitting!");
  }

  VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  int mipWidth = width;
  int mipHeight = height;

  for (uint32_t i = 1; i < mipLevel; i++) {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    VkImageBlit blit{};
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = i - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = i;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;

    vkCmdBlitImage(commandBuffer,
                   image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &blit,
                   filter);

    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    mipWidth = std::max(1, mipWidth / 2);
    mipHeight = std::max(1, mipHeight / 2);
  }

  endSingleTimeCommands(context, commandBuffer);
}

VkSampleCountFlagBits VulkanUtils::getMaxUsableSampleCount(VkPhysicalDevice physicalDevice) {
  VkPhysicalDeviceProperties physicalDeviceProperties;
  vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

  VkSampleCountFlags counts = std::min(
      physicalDeviceProperties.limits.framebufferColorSampleCounts,
      physicalDeviceProperties.limits.framebufferDepthSampleCounts
  );

  if (counts & VK_SAMPLE_COUNT_64_BIT) {
    return VK_SAMPLE_COUNT_64_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_32_BIT) {
    return VK_SAMPLE_COUNT_32_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_16_BIT) {
    return VK_SAMPLE_COUNT_16_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_8_BIT) {
    return VK_SAMPLE_COUNT_8_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_4_BIT) {
    return VK_SAMPLE_COUNT_4_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_2_BIT) {
    return VK_SAMPLE_COUNT_2_BIT;
  }

  return VK_SAMPLE_COUNT_1_BIT;
}

VkShaderModule VulkanUtils::createShaderModule(VkDevice device,
                                               const uint32_t *code,
                                               uint32_t size) {
  VkShaderModuleCreateInfo shaderInfo = {};
  shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderInfo.pCode = code;
  shaderInfo.codeSize = size;
  VkShaderModule shader;
  VK_CHECK(vkCreateShaderModule(device, &shaderInfo, nullptr, &shader), "Create shader module failed\n");
  return shader;
}

void VulkanUtils::bindUniformBuffer(VkDevice device,
                                    VkDescriptorSet descriptorSet,
                                    int binding,
                                    VkBuffer buffer,
                                    VkDeviceSize offset,
                                    VkDeviceSize size
) {
  VkDescriptorBufferInfo descriptorBufferInfo = {};
  descriptorBufferInfo.buffer = buffer;
  descriptorBufferInfo.offset = offset;
  descriptorBufferInfo.range = size;

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = descriptorSet;
  descriptorWrite.dstBinding = binding;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pBufferInfo = &descriptorBufferInfo;

  // TODO: not optimal, probably should be refactored to a Binder class,
  // i.e. it's better to collect all descriptor writes before the call
  vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

void VulkanUtils::bindCombinedImageSampler(
    VkDevice device,
    VkDescriptorSet descriptorSet,
    int binding,
    VkImageView imageView,
    VkSampler sampler
) {
  VkDescriptorImageInfo descriptorImageInfo = {};
  descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  descriptorImageInfo.imageView = imageView;
  descriptorImageInfo.sampler = sampler;

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = descriptorSet;
  descriptorWrite.dstBinding = binding;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pImageInfo = &descriptorImageInfo;

  // TODO: not optimal, probably should be refactored to a Binder class,
  // i.e. it's better to collect all descriptor writes before the call
  vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

  //
}

void VulkanUtils::endSingleTimeCommands(const Device *context, VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = 0;

  VkFence fence{};
  VK_CHECK(vkCreateFence(context->LogicDevice(), &fenceInfo, nullptr, &fence), "Create Fence Failed\n");

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  VK_CHECK(vkQueueSubmit(context->GraphicsQueue(), 1, &submitInfo, fence), "failed to submit command buffer");
  VK_CHECK(vkWaitForFences(context->LogicDevice(), 1, &fence, VK_TRUE, 10000000000), "Wait for Fence failed");
  vkDestroyFence(context->LogicDevice(), fence, nullptr);
  vkFreeCommandBuffers(context->LogicDevice(), context->CommandPool(), 1, &commandBuffer);
}

VkFormat VulkanUtils::selectOptimalSupportedImageFormat(const VkPhysicalDevice &physicalDevice,
                                                        const std::vector<VkFormat> &candiates,
                                                        VkImageTiling tiling,
                                                        VkFormatFeatureFlags features) {
  for (VkFormat format: candiates) {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
    if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
      return format;

    if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
      return format;
    }
    std::cout << format << "\n";
  }

  TH_WITH_MSG(true, "can not find support format");
}

VkFormat VulkanUtils::selectOptimalImageFormat(const VkPhysicalDevice &physicalDevice) {
  return selectOptimalSupportedImageFormat(physicalDevice,
                                           {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                            VK_FORMAT_D24_UNORM_S8_UINT},
                                           VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}

/*
 */
bool VulkanUtils::checkInstanceValidationLayers(
    const std::vector<const char *> &requiredLayers,
    bool verbose
) {
  uint32_t availableLayerCount = 0;
  vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(availableLayerCount);
  vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());

  for (const char *requiredLayer: requiredLayers) {
    bool supported = false;
    for (const VkLayerProperties &layer: availableLayers) {
      if (strcmp(requiredLayer, layer.layerName) == 0) {
        supported = true;
        break;
      }
    }

    if (!supported) {
      if (verbose)
        std::cerr << requiredLayer << " is not supported" << std::endl;

      return false;
    }

    if (verbose)
      std::cout << "Have " << requiredLayer << std::endl;
  }

  return true;
}

bool VulkanUtils::checkInstanceExtensions(
    const std::vector<const char *> &requiredExtensions,
    bool verbose
) {
  uint32_t availableExtensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

  for (const char *requiredExtension: requiredExtensions) {
    bool supported = false;
    for (const VkExtensionProperties &availableExtension: availableExtensions) {
      if (strcmp(requiredExtension, availableExtension.extensionName) == 0) {
        supported = true;
        break;
      }
    }

    if (!supported) {
      if (verbose)
        std::cerr << requiredExtension << " is not supported" << std::endl;

      return false;
    }

    if (verbose)
      std::cout << "Have " << requiredExtension << std::endl;
  }

  return true;
}

bool VulkanUtils::checkPhysicalDeviceExtensions(
    VkPhysicalDevice physicalDevice,
    const std::vector<const char *> &requiredExtensions,
    bool verbose
) {
  uint32_t availableDeviceExtensionCount = 0;
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableDeviceExtensionCount, nullptr);

  std::vector<VkExtensionProperties> availableDeviceExtensions(availableDeviceExtensionCount);
  vkEnumerateDeviceExtensionProperties(physicalDevice,
                                       nullptr,
                                       &availableDeviceExtensionCount,
                                       availableDeviceExtensions.data());

  for (const char *requiredExtension: requiredExtensions) {
    bool supported = false;
    for (const VkExtensionProperties &availableDeviceExtension: availableDeviceExtensions) {
      if (strcmp(requiredExtension, availableDeviceExtension.extensionName) == 0) {
        supported = true;
        break;
      }
    }

    if (!supported) {
      if (verbose)
        std::cerr << requiredExtension << " is not supported on this physical device" << std::endl;

      return false;
    }

    if (verbose)
      std::cout << "Have " << requiredExtension << std::endl;
  }

  return true;
}

void VulkanUtils::createDeviceLocalBuffer(const Device *context,
                                          VkDeviceSize size,
                                          const void *data,
                                          VkBufferUsageFlags usageFlags,
                                          VkBuffer &buffer,
                                          VkDeviceMemory &memory) {
  VulkanUtils::createBuffer(
      context,
      size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlags,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      buffer,
      memory
  );

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  VulkanUtils::createBuffer(context, size,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            stagingBuffer,
                            stagingBufferMemory);

  void *staging_buffer_data = nullptr;
  vkMapMemory(context->LogicDevice(), stagingBufferMemory, 0, size, 0, &staging_buffer_data);
  memcpy(staging_buffer_data, data, size);
  vkUnmapMemory(context->LogicDevice(), stagingBufferMemory);

  VulkanUtils::copyBuffer(context, stagingBuffer, buffer, size);

  vkDestroyBuffer(context->LogicDevice(), stagingBuffer, nullptr);
  vkFreeMemory(context->LogicDevice(), stagingBufferMemory, nullptr);
}

bool VulkanUtils::isDepthFormat(VkFormat format) {
  switch (format) {
    case VK_FORMAT_D16_UNORM: return true;
    case VK_FORMAT_D16_UNORM_S8_UINT: return true;
    case VK_FORMAT_D24_UNORM_S8_UINT: return true;
    case VK_FORMAT_D32_SFLOAT: return true;
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return true;
    default: return false;
  }
}

}