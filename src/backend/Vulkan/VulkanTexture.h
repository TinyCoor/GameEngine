//
// Created by y123456 on 2021/10/16.
//

#ifndef GAMEENGINE_VULKANTEXTURE_H
#define GAMEENGINE_VULKANTEXTURE_H
#include <string>
#include "driver.h"
namespace render::backend::vulkan {

struct Texture : public render::backend::Texture {
    VkImage image{VK_NULL_HANDLE};
    VkImageView view{VK_NULL_HANDLE};
    VkSampler sampler{VK_NULL_HANDLE};
    VmaAllocation memory {VK_NULL_HANDLE};
    VkImageType type{VK_IMAGE_TYPE_2D};
    VkFormat format{VK_FORMAT_R8G8B8A8_UNORM};
    size_t width{0};
    size_t height{0};
    size_t depth{0};
    uint8_t num_layers{0};
    uint8_t num_mipmaps{0};
    VkImageTiling tiling{VK_IMAGE_TILING_OPTIMAL};
    VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};
    VkImageCreateFlags flags{0};
};

class VulkanTexture {
public:
  VulkanTexture(render::backend::Driver *driver):driver(driver) { }
  ~VulkanTexture();

  inline const render::backend::Texture *getTexture() const { return texture; }
  void create2D(render::backend::Format format, int width, int height, int num_mips);
  void createCube(render::backend::Format format, int width, int height, int num_mips);

  VkImage getImage() const ;
  VkFormat getImageFormat() const ;
  VkImageView getImageView() const ;
  VkSampler getSampler() const;

  inline size_t getWidth() const { return width; }
  inline int getWidth(int mip) const { return std::max<int>(1, width / (1 << mip)); }
  inline size_t getHeight() const { return height; }
  inline int getHeight(int mip) const { return std::max<int>(1, height / (1 << mip)); }
  inline size_t getNumLayers() const { return layers; }
  inline size_t getNumMiplevels() const { return mip_levels; }

  bool import(const char* file);
  void clearGPUData();
  void clearCPUData();

private:
  render::backend::Driver *driver {nullptr};
  unsigned char *pixels = nullptr;
  int width = 0;
  int height = 0;
  int mip_levels = 0;
  int layers = 0;

  render::backend::Texture *texture {nullptr};
};
}

#endif //GAMEENGINE_VULKANTEXTURE_H
