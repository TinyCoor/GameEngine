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

static int deduceChannels(VkFormat format){
    switch (format) {
        case VK_FORMAT_R32_SFLOAT: return 1;
        case VK_FORMAT_R32G32_SFLOAT: return 2;
        case VK_FORMAT_R32G32B32_SFLOAT:return 3;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return 4;
        case VK_FORMAT_R8G8B8A8_UNORM: return 4;
        default:{
           throw std::runtime_error("not support");
        }
    }
}

static int deducePixelSize(VkFormat format){
    switch (format) {
        case VK_FORMAT_R32_SFLOAT: return sizeof(float);
        case VK_FORMAT_R32G32_SFLOAT: return sizeof(float) *2;
        case VK_FORMAT_R32G32B32_SFLOAT:return sizeof(float) *3;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return sizeof(float) *4;
        case VK_FORMAT_R8G8B8A8_UNORM: return 4;
        default:{
            throw std::runtime_error("not support");
        }
    }
}
static VkImageTiling deduceTiling(VkFormat format){
    switch (format) {
        case VK_FORMAT_R32_SFLOAT: return VK_IMAGE_TILING_LINEAR;
        case VK_FORMAT_R32G32_SFLOAT: return VK_IMAGE_TILING_LINEAR;
        case VK_FORMAT_R32G32B32_SFLOAT:return VK_IMAGE_TILING_LINEAR;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return VK_IMAGE_TILING_LINEAR;
        case VK_FORMAT_R8G8B8A8_UNORM: return VK_IMAGE_TILING_OPTIMAL;
        default:{
            throw std::runtime_error("not support");
        }
    }
}

VulkanTexture::~VulkanTexture() {
    //TODO
    // clearCPUData();
    // clearGPUData();
}


void VulkanTexture::uploadToGPU(VkFormat format,VkImageTiling tiling,size_t size) {
    imageFormat = format;
    VkDeviceSize imageSize = width * height * size;
    TH_WITH_MSG(!pixels,"failed to load texture image!");

    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMemory{};
    vulkanUtils:: createBuffer(context,imageSize,
                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               stagingBuffer,
                               stagingBufferMemory);

    void* data;
    vkMapMemory(context.device_, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(context.device_, stagingBufferMemory);

    vulkanUtils::createImage2D(context,width,
                               height,
                               mipLevels,
                               VK_SAMPLE_COUNT_1_BIT,
                               imageFormat,
                               tiling,
                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               image, imageMemory);

    //Prepare the image for transfer
    vulkanUtils::transitionImageLayout(context,image,imageFormat,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       0,mipLevels);

    // copt to gpu
    vulkanUtils::copyBufferToImage(context,
                                   stagingBuffer,image,
                                   width,height);

    //Generate mipmaps on GPU
    vulkanUtils::generateImage2DMipMaps(context,image,width,height,mipLevels,
                                        imageFormat,VK_FILTER_LINEAR);

    //Prepare the image for shader access
    vulkanUtils::transitionImageLayout(context, image,
                                       imageFormat,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                       0,mipLevels);

    //clean up
    vkDestroyBuffer(context.device_,stagingBuffer, nullptr);
    vkFreeMemory(context.device_, stagingBufferMemory, nullptr);

    //create ImageView
    imageView =vulkanUtils::createImageView(context,
                                              image,
                                              imageFormat,
                                              VK_IMAGE_ASPECT_COLOR_BIT,
                                              0,mipLevels,0,layers);
    imageSampler= vulkanUtils::createSampler2D(context,mipLevels);

}


void VulkanTexture::clearGPUData() {
    vkDestroySampler(context.device_,imageSampler, nullptr);
    vkDestroyImageView(context.device_,imageView, nullptr);
    vkDestroyImage(context.device_, image, nullptr);
    vkFreeMemory(context.device_, imageMemory, nullptr);

    image = VK_NULL_HANDLE;
    imageView = VK_NULL_HANDLE;
    imageSampler = VK_NULL_HANDLE;
    imageMemory= VK_NULL_HANDLE;
    imageFormat = VK_FORMAT_UNDEFINED;
}

void VulkanTexture::clearCPUData() {
    if(pixels != nullptr){
        delete[] pixels;
        pixels = nullptr;
        width = height = channels =0;
    }
}

bool  VulkanTexture::loadFromFile(const std::string &path) {
    stbi_uc* stb_pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    channels  = 4;
    if(!stb_pixels){
        std::cerr<< "load file failed:" << path <<'\n';
        return false;
    }

    mipLevels =static_cast<int>( std::floor(std::log2(std::max(width,height)))+ 1);
    layers = 1;

    clearCPUData();
    size_t pixel_size= channels *sizeof(stbi_uc);
    size_t size = width* height * pixel_size;
    pixels = new unsigned char[size];
    memcpy(pixels,stb_pixels,size);
    stbi_image_free(stb_pixels);
    stb_pixels= nullptr;
    //
    clearGPUData();
    uploadToGPU(VK_FORMAT_R8G8B8A8_UNORM,VK_IMAGE_TILING_OPTIMAL,pixel_size);
    return true;
}

bool VulkanTexture::loadHDRFromFile(const std::string& path) {
    stbi_set_flip_vertically_on_load(true);
    float* stb_pixels = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
    if(!stb_pixels){
        std::cerr<< "load file failed:" << path <<'\n';
        return false;
    }

    mipLevels = 1;
    layers = 1;

    size_t pixel_size =  channels * sizeof(float);
    size_t size = width *height * pixel_size;
    clearCPUData();

    pixels = new unsigned char[size];
    memcpy(pixels,stb_pixels,size);
    stbi_image_free(stb_pixels);
    stb_pixels = nullptr;
    clearGPUData();
    VkFormat format = VK_FORMAT_UNDEFINED;
    switch (channels) {
        case 1:format = VK_FORMAT_R32_SFLOAT;break;
        case 2: format = VK_FORMAT_R32G32_SFLOAT;break;
        case 3: format= VK_FORMAT_R32G32B32_SFLOAT;break;
    }

    uploadToGPU(format,VK_IMAGE_TILING_LINEAR,pixel_size);
    return true;
}

void VulkanTexture::createCube(VkFormat format,int w,int h,int numMipLevels)
{
    width = w;
    height = h;
    mipLevels = numMipLevels;
    layers = 6;
    imageFormat=format;
    channels  = deduceChannels(format) ;//??
    VkImageTiling tiling = deduceTiling(format);

    vulkanUtils::createCubeImage(
            context,
            width,height,mipLevels,
            VK_SAMPLE_COUNT_1_BIT,
            format,
            tiling,
            VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            image,
            imageMemory
            );

    //Prepare the image for transfer
    vulkanUtils::transitionImageLayout(context,
                                       image,
                                       format,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                       0,
                                       mipLevels,
                                       0,
                                       layers);



    imageView =vulkanUtils::createImageView(context,image,format,VK_IMAGE_ASPECT_COLOR_BIT,0,mipLevels,0,layers);
    imageSampler= vulkanUtils::createSampler2D(context,mipLevels);

}

