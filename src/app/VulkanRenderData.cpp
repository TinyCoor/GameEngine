//
// Created by y123456 on 2021/10/12.
//

#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "VulkanRenderData.h"
#include "Macro.h"



void RenderData::init(const std::string& vertShaderFile,
                      const std::string& fragShaderFile,
                      const std::string& textureFile,
                      const std::string& modelFile){
    vertShader = createShader(vertShaderFile);
    fragShader = createShader(fragShaderFile);
    mesh.loadFromFile(modelFile);
    createImage(textureFile);
}

void RenderData::shutdown(){

    vkDestroySampler(context.device_,textureImageSampler, nullptr);
    vkDestroyImageView(context.device_,textureImageView, nullptr);
    vkDestroyImage(context.device_, textureImage, nullptr);
    vkFreeMemory(context.device_, textureImageMemory, nullptr);

    vkDestroyShaderModule(context.device_,vertShader,nullptr);
    vkDestroyShaderModule(context.device_,fragShader,nullptr);
    vertShader = VK_NULL_HANDLE;
    fragShader = VK_NULL_HANDLE;

}

VkShaderModule RenderData::createShader(const std::string &path) const {
    std::vector<char> vertex_code = vulkanUtils::readFile(path);
    VkShaderModuleCreateInfo shaderInfo={};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.pCode = reinterpret_cast<const uint32_t *>(vertex_code.data());
    shaderInfo.codeSize = static_cast<uint32_t> (vertex_code.size());
    VkShaderModule shader;
    VK_CHECK(vkCreateShaderModule(context.device_,&shaderInfo, nullptr,&shader),"Create shader module failed\n");
    return shader;
}



void RenderData::createImage(const std::string& path){
    int texWidth, texHeight, texChannels;
    // pixel data will have alpha channel even if the original image
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels,  STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

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

    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    vulkanUtils::createImage2D(context,texWidth,
                               texHeight,
                               format,
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               textureImage, textureImageMemory);

    //Copy to GPU
    vulkanUtils::transitionImageLayout(context,
                          textureImage,
                          format,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vulkanUtils::copyBufferToImage(context,
                               stagingBuffer,textureImage,
                               texWidth,texHeight);

    vulkanUtils::transitionImageLayout(context, textureImage,
                                       format,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    //clean up
    vkDestroyBuffer(context.device_,stagingBuffer, nullptr);
    vkFreeMemory(context.device_, stagingBufferMemory, nullptr);
    stbi_image_free(pixels);
    pixels = nullptr;

    //create ImageView
    textureImageView =vulkanUtils::createImage2DView(context,
                                                     textureImage,
                                                     format,
                                                     VK_IMAGE_ASPECT_COLOR_BIT);
    textureImageSampler= vulkanUtils::createSampler2D(context);

}
