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

VulkanTexture::~VulkanTexture() {
    //TODO
    // clearCPUData();
    // clearGPUData();
}


void VulkanTexture::uploadToGPU() {

    //TODO Support Other Image Format

    VkDeviceSize imageSize = width * height * 4;
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
                               format,
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               image, imageMemory);

    //Prepare the image for transfer
    vulkanUtils::transitionImageLayout(context,image,mipLevels,format,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // copt to gpu
    vulkanUtils::copyBufferToImage(context,
                                   stagingBuffer,image,
                                   width,height);

    //Generate mipmaps on GPU
    vulkanUtils::generateImage2DMipMaps(context,image,width,height,mipLevels,
                                        format,VK_FILTER_LINEAR);

    //Prepare the image for shader access
    vulkanUtils::transitionImageLayout(context, image,
                                       mipLevels,
                                       format,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    //clean up
    vkDestroyBuffer(context.device_,stagingBuffer, nullptr);
    vkFreeMemory(context.device_, stagingBufferMemory, nullptr);

    //create ImageView
    imageView =vulkanUtils::createImage2DView(context,
                                              image,
                                              mipLevels,
                                              format,
                                              VK_IMAGE_ASPECT_COLOR_BIT);
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
}

void VulkanTexture::clearCPUData() {
    if(pixels != nullptr){
        delete[] pixels;
        pixels = nullptr;
        width = height = channels =0;
    }
}

bool  VulkanTexture::loadFromFile(const std::string &path) {

    //TODO Support Other Format
    stbi_uc* stb_pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if(!stb_pixels){
        std::cerr<< "load file failed:" << path <<'\n';
        return false;
    }

    mipLevels =static_cast<int>( std::floor(std::log2(std::max(width,height)))+ 1);

    clearCPUData();

    size_t size = width* height *4;
    pixels = new unsigned char[size];
    memcpy(pixels,stb_pixels,size);
    stbi_image_free(stb_pixels);
    stb_pixels= nullptr;
    //
    clearGPUData();
    uploadToGPU();

    return true;
}

