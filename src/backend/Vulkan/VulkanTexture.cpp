//
// Created by y123456 on 2021/10/16.
//

#include "VulkanTexture.h"
#include "VulkanUtils.h"
#include "Macro.h"
#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

namespace render::backend::vulkan {

static int deduceChannels(VkFormat format) {
  switch (format) {
  case VK_FORMAT_R8_UNORM: return 1;
  case VK_FORMAT_R8G8_UNORM: return 2;
  case VK_FORMAT_R8G8B8_UNORM: return 3;
  case VK_FORMAT_R8G8B8A8_UNORM: return 4;
  case VK_FORMAT_R16_SFLOAT: return 1;
  case VK_FORMAT_R16G16_SFLOAT: return 2;
  case VK_FORMAT_R16G16B16_SFLOAT: return 3;
  case VK_FORMAT_R16G16B16A16_SFLOAT: return 4;
  case VK_FORMAT_R32_SFLOAT: return 1;
  case VK_FORMAT_R32G32_SFLOAT: return 2;
  case VK_FORMAT_R32G32B32_SFLOAT: return 3;
  case VK_FORMAT_R32G32B32A32_SFLOAT: return 4;
  default: {
    throw std::runtime_error("deduceChannels: not support");
  }
  }
}

static VkFormat deduceFormat(size_t pixel_size, int channels) {
  if (pixel_size == sizeof(stbi_uc)) {
    switch (channels) {
    case 1: return VK_FORMAT_R8_UNORM;
    case 2: return VK_FORMAT_R8G8_UNORM;
    case 3: return VK_FORMAT_R8G8B8_UNORM;
    case 4: return VK_FORMAT_R8G8B8A8_UNORM;
    }
  }

  if (pixel_size == sizeof(float)) {
    switch (channels) {
    case 1: return VK_FORMAT_R32_SFLOAT;
    case 2: return VK_FORMAT_R32G32_SFLOAT;
    case 3: return VK_FORMAT_R32G32B32_SFLOAT;
    case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
  }
  return VK_FORMAT_UNDEFINED;
}

static VkImageTiling deduceTiling(VkFormat format) {
  switch (format) {
  case VK_FORMAT_R8_UNORM: return VK_IMAGE_TILING_OPTIMAL;
  case VK_FORMAT_R8G8_UNORM: return VK_IMAGE_TILING_OPTIMAL;
  case VK_FORMAT_R8G8B8_UNORM: return VK_IMAGE_TILING_OPTIMAL;
  case VK_FORMAT_R8G8B8A8_UNORM: return VK_IMAGE_TILING_OPTIMAL;
  case VK_FORMAT_R16_SFLOAT: return VK_IMAGE_TILING_LINEAR;
  case VK_FORMAT_R16G16_SFLOAT: return VK_IMAGE_TILING_LINEAR;
  case VK_FORMAT_R16G16B16_SFLOAT: return VK_IMAGE_TILING_LINEAR;
  case VK_FORMAT_R16G16B16A16_SFLOAT: return VK_IMAGE_TILING_LINEAR;
  case VK_FORMAT_R32_SFLOAT: return VK_IMAGE_TILING_LINEAR;
  case VK_FORMAT_R32G32_SFLOAT: return VK_IMAGE_TILING_LINEAR;
  case VK_FORMAT_R32G32B32_SFLOAT: return VK_IMAGE_TILING_LINEAR;
  case VK_FORMAT_R32G32B32A32_SFLOAT: return VK_IMAGE_TILING_LINEAR;
  default: {
    throw std::runtime_error("not support");
  }
  }
}

VulkanTexture::~VulkanTexture() {
  clearCPUData();
  clearGPUData();
}

void VulkanTexture::uploadToGPU(VkFormat format, VkImageTiling tiling, size_t imageSize) {
  imageFormat = format;

  TH_WITH_MSG(!pixels, "failed to load texture image!");

  VkBuffer stagingBuffer{};
  VkDeviceMemory stagingBufferMemory{};
  VulkanUtils::createBuffer(context, imageSize,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            stagingBuffer,
                            stagingBufferMemory);

  void *data;
  vkMapMemory(context->Device(), stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(context->Device(), stagingBufferMemory);

  VulkanUtils::createImage2D(context, width,
                             height,
                             mipLevels,
                             VK_SAMPLE_COUNT_1_BIT,
                             imageFormat,
                             tiling,
                             VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                 | VK_IMAGE_USAGE_SAMPLED_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             image, imageMemory);

  //Prepare the image for transfer
  VulkanUtils::transitionImageLayout(context, image, imageFormat,
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                     0, mipLevels);

  // copt to gpu
  VulkanUtils::copyBufferToImage(context,
                                 stagingBuffer, image,
                                 width, height);

  //Generate mipmaps on GPU
  //BUG
  VulkanUtils::generateImage2DMipMaps(context, image, width, height, mipLevels,
                                      imageFormat, VK_FILTER_LINEAR);

  //Prepare the image for shader access
  VulkanUtils::transitionImageLayout(context, image,
                                     imageFormat,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                     0, mipLevels);

  //clean up
  vkDestroyBuffer(context->Device(), stagingBuffer, nullptr);
  vkFreeMemory(context->Device(), stagingBufferMemory, nullptr);

  //create ImageView
  imageView = VulkanUtils::createImageView(context->Device(),
                                           image,
                                           imageFormat,
                                           VK_IMAGE_ASPECT_COLOR_BIT,
                                           VK_IMAGE_VIEW_TYPE_2D,
                                           0, mipLevels, 0, layers);

  imageSampler = VulkanUtils::createSampler(context->Device(), 0, mipLevels);

  // Create mip image views
  mipViews.resize(mipLevels);
  for (int i = 0; i < mipLevels; i++) {
    mipViews[i] = VulkanUtils::createImageView(
        context->Device(),
        image,
        imageFormat,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_VIEW_TYPE_2D,
        i, 1,
        0, layers
    );
  }

}

void VulkanTexture::clearGPUData() {
  vkDestroySampler(context->Device(), imageSampler, nullptr);
  imageSampler = VK_NULL_HANDLE;
  vkDestroyImageView(context->Device(), imageView, nullptr);
  imageView = VK_NULL_HANDLE;
  vkDestroyImage(context->Device(), image, nullptr);
  image = VK_NULL_HANDLE;
  vkFreeMemory(context->Device(), imageMemory, nullptr);
  imageMemory = VK_NULL_HANDLE;

  for (int i = 0; i < mipViews.size(); ++i) {
    vkDestroyImageView(context->Device(), mipViews[i], nullptr);
  }
  mipViews.clear();

  imageFormat = VK_FORMAT_UNDEFINED;
}

void VulkanTexture::clearCPUData() {
  if (pixels != nullptr) {
    delete[] pixels;
    pixels = nullptr;
    width = height = channels = 0;
  }
}

bool VulkanTexture::loadFromFile(const std::string &path) {
  //TODO
  if (stbi_info(path.c_str(), nullptr, nullptr, nullptr) == 0) {
    std::cerr << "loadFromFile: unsupported texture format " << path << '\n';
    return false;
  }

  void *stb_pixels = nullptr;
  size_t pixelSize = 0;

  if (stbi_is_hdr(path.c_str())) {
    stb_pixels = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
    pixelSize = sizeof(float);
    mipLevels = 1; //TODO
  } else {
    stb_pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    pixelSize = sizeof(stbi_uc);
    channels = 4;
    mipLevels = static_cast<int>(std::floor(std::log2(std::max(width, height))) + 1);
  }

  //TODO RGB convert rbga
  if (!stb_pixels) {
    std::cerr << "load file failed:" << path << '\n';
    return false;
  }

  layers = 1;
  VkFormat format = deduceFormat(pixelSize, channels);
  VkImageTiling tiling = deduceTiling(format);

  clearCPUData();
  size_t imageSize = width * height * channels * pixelSize;
  pixels = new unsigned char[imageSize];

  memcpy(pixels, stb_pixels, imageSize);

  stbi_image_free(stb_pixels);
  stb_pixels = nullptr;

  clearGPUData();
  uploadToGPU(format, tiling, imageSize);
  return true;
}

void VulkanTexture::createCube(VkFormat format, int w, int h, int numMipLevels) {
  width = w;
  height = h;
  mipLevels = numMipLevels;
  layers = 6;
  imageFormat = format;
  channels = deduceChannels(format);
  VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;

  VulkanUtils::createCubeImage(
      context,
      width, height, mipLevels,
      VK_SAMPLE_COUNT_1_BIT,
      format,
      tiling,
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      image,
      imageMemory
  );

  //Prepare the image for transfer
  VulkanUtils::transitionImageLayout(context,
                                     image,
                                     format,
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                     0,
                                     mipLevels,
                                     0,
                                     layers);

  imageView = VulkanUtils::createImageView(context->Device(),
                                           image,
                                           format,
                                           VK_IMAGE_ASPECT_COLOR_BIT,
                                           VK_IMAGE_VIEW_TYPE_CUBE,
                                           0, mipLevels,
                                           0, layers);
  imageSampler = VulkanUtils::createSampler(context->Device(), 0, mipLevels);

  // Create mip image views
  mipViews.resize(mipLevels);
  for (int i = 0; i < mipLevels; i++) {
    mipViews[i] = VulkanUtils::createImageView(
        context->Device(),
        image,
        imageFormat,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_VIEW_TYPE_CUBE,
        i, 1,
        0, layers
    );
  }
}

void VulkanTexture::create2D(VkFormat format, int w, int h, int numMipLevels) {
  width = w;
  height = h;
  mipLevels = numMipLevels;
  layers = 1;
  imageFormat = format;
  channels = deduceChannels(format);
  VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;

  VulkanUtils::createImage2D(
      context,
      width, height, mipLevels,
      VK_SAMPLE_COUNT_1_BIT,
      format,
      tiling,
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      image,
      imageMemory
  );

  //Prepare the image for transfer
  VulkanUtils::transitionImageLayout(context,
                                     image,
                                     format,
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                     0,
                                     mipLevels,
                                     0,
                                     layers);

  imageView = VulkanUtils::createImageView(context->Device(),
                                           image,
                                           format,
                                           VK_IMAGE_ASPECT_COLOR_BIT,
                                           VK_IMAGE_VIEW_TYPE_2D,
                                           0, mipLevels,
                                           0, layers);
  imageSampler = VulkanUtils::createSampler(context->Device(), 0, mipLevels);

  // Create mip image views
  mipViews.resize(mipLevels);
  for (int i = 0; i < mipLevels; i++) {
    mipViews[i] = VulkanUtils::createImageView(
        context->Device(),
        image,
        imageFormat,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_VIEW_TYPE_2D,
        i, 1,
        0, layers
    );
  }
}
}
