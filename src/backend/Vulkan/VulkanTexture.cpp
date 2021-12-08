//
// Created by y123456 on 2021/10/16.
//

#include "VulkanTexture.h"
#include "VulkanUtils.h"
#include "Macro.h"
#include <stdexcept>
#include <cassert>
#include <stb_image.h>
#include <iostream>
#include <cmath>
#include <cstring>

namespace render::backend::vulkan {

static render::backend::Format deduceFormat(size_t pixelSize, int channels) {
  assert(channels > 0 && channels <= 4);

  if (pixelSize == sizeof(stbi_uc)) {
    switch (channels) {
    case 1: return render::backend::Format::R8_UNORM;
    case 2: return render::backend::Format::R8G8_UNORM;
    case 3: return render::backend::Format::R8G8B8_UNORM;
    case 4: return render::backend::Format::R8G8B8A8_UNORM;
    }
  }

  if (pixelSize == sizeof(float)) {
    switch (channels) {
    case 1: return render::backend::Format::R32_SFLOAT;
    case 2: return render::backend::Format::R32G32_SFLOAT;
    case 3: return render::backend::Format::R32G32B32_SFLOAT;
    case 4: return render::backend::Format::R32G32B32A32_SFLOAT;
    }
  }

  return render::backend::Format::UNDEFINED;
}

VulkanTexture::~VulkanTexture() {
  clearCPUData();
  clearGPUData();
}

void VulkanTexture::clearGPUData() {
  driver->destroyTexture(texture);
  texture = nullptr;
}

void VulkanTexture::clearCPUData() {
  if (pixels != nullptr) {
    delete[] pixels;
    pixels = nullptr;
    width = height = 0;
  }
}

bool VulkanTexture::loadFromFile(const std::string &path) {
  if (stbi_info(path.c_str(), nullptr, nullptr, nullptr) == 0) {
    std::cerr << "VulkanTexture::loadFromFile(): unsupported image format for \"" << path << "\" file" << std::endl;
    return false;
  }

  void *stb_pixels = nullptr;
  size_t pixel_size = 0;
  int channels = 0;

  if (stbi_is_hdr(path.c_str())) {
    stb_pixels = stbi_loadf(path.c_str(), &width, &height, &channels, STBI_default);
    pixel_size = sizeof(float);
  } else {
    stb_pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_default);
    pixel_size = sizeof(stbi_uc);
  }

  if (!stb_pixels) {
    std::cerr << "VulkanTexture::loadFromFile(): " << stbi_failure_reason() << std::endl;
    return false;
  }

  layers = 1;
  mip_levels = static_cast<int>(std::floor(std::log2(std::max(width, height))) + 1);

  bool convert = false;
  if (channels == 3) {
    channels = 4;
    convert = true;
  }

  size_t image_size = width * height * channels * pixel_size;
  if (pixels != nullptr)
    delete[] pixels;

  pixels = new unsigned char[image_size];

  // As most hardware doesn't support rgb textures, convert it to rgba
  if (convert) {
    size_t numPixels = width * height;
    size_t stride = pixel_size * 3;

    unsigned char *d = pixels;
    unsigned char *s = reinterpret_cast<unsigned char *>(stb_pixels);

    for (size_t i = 0; i < numPixels; i++) {
      memcpy(d, s, stride);
      s += stride;
      d += stride;

      memset(d, 0, pixel_size);
      d += pixel_size;
    }
  } else
    memcpy(pixels, stb_pixels, image_size);

  stbi_image_free(stb_pixels);
  stb_pixels = nullptr;

  render::backend::Format format = deduceFormat(pixel_size, channels);

  // Upload CPU data to GPU
  clearGPUData();

  texture = driver->createTexture2D(width, height, mip_levels, format, pixels);
  driver->generateTexture2DMipmaps(texture);

  // TODO: should we clear CPU data after uploading it to the GPU?

  return true;
}

void VulkanTexture::createCube(render::backend::Format format, int w, int h, int numMipLevels) {
  width = w;
  height = h;
  mip_levels = numMipLevels;
  layers = 6;
  texture = driver->createTextureCube(w, h, mip_levels, format);
}

void VulkanTexture::create2D(render::backend::Format format, int w, int h, int numMipLevels) {
  width = w;
  height = h;
  mip_levels = numMipLevels;
  layers = 1;
  texture = driver->createTexture2D(width, height, mip_levels, format);
}
VkImage VulkanTexture::getImage() const {
  if (texture == nullptr)
    return VK_NULL_HANDLE;

  return static_cast<render::backend::vulkan::Texture *>(texture)->image;

}
VkFormat VulkanTexture::getImageFormat() const {
  if (texture == nullptr)
    return VK_FORMAT_UNDEFINED;

  return static_cast<render::backend::vulkan::Texture *>(texture)->format;
}
VkImageView VulkanTexture::getImageView() const {
  if (texture == nullptr)
    return VK_NULL_HANDLE;

  return static_cast<render::backend::vulkan::Texture *>(texture)->view;
}
VkSampler VulkanTexture::getSampler() const {
  if (texture == nullptr)
    return VK_NULL_HANDLE;

  return static_cast<render::backend::vulkan::Texture *>(texture)->sampler;
}
}