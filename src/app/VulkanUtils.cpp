//
// Created by y123456 on 2021/10/13.
//

#include "VulkanUtils.h"
#include "VulkanContext.h"
#include "Macro.h"
#include <stdexcept>
#include <fstream>
#include <cassert>
#include <iostream>
#include <cstring>

QueueFamilyIndices VulkanUtils::fetchFamilyIndices(VkPhysicalDevice& physcalDevice,VkSurfaceKHR & surface) {
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physcalDevice,&queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physcalDevice,&queueCount,queueFamilies.data());
    QueueFamilyIndices indices{};
    for (int i = 0; i< queueFamilies.size();++i) {

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = std::make_optional(i);
        }

        VkBool32  presentSupport= false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physcalDevice,i,surface,&presentSupport);
        if(queueFamilies[i].queueCount > 0 && presentSupport){
            indices.presentFamily = std::make_optional(i);
        }
        if(indices.isComplete()){
            break;
        }
    }
    return indices;
}

void VulkanUtils::createBuffer(const VulkanContext* context,
                               VkDeviceSize size,
                               VkBufferUsageFlags usageFlags,
                               VkMemoryPropertyFlags memoryFlags,
                               VkBuffer& buffer,
                               VkDeviceMemory& memory) {
    //Create Vertex Buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK(vkCreateBuffer(context->device, &bufferInfo, nullptr, &buffer),
             "failed to create vertex buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context->device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(context->physicalDevice,memRequirements.memoryTypeBits, memoryFlags);
    VK_CHECK(vkAllocateMemory(context->device, &allocInfo, nullptr, &memory) ,"failed to allocate vertex buffer memory!");
    VK_CHECK( vkBindBufferMemory(context->device, buffer, memory, 0),"Bind Buffer VertexBuffer Failed");

}



uint32_t VulkanUtils::findMemoryType(const VkPhysicalDevice physicalDevice,
                                     uint32_t typeFilter,
                                     VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        uint32_t memoryPropertiy   = memProperties.memoryTypes[i].propertyFlags;
        if (typeFilter & (1 << i) && (memoryPropertiy & properties) == properties){
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanUtils::copyBuffer(const VulkanContext* context,
                             VkBuffer srcBuffer,
                             VkBuffer dstBuffer,
                             VkDeviceSize size) {


    auto commandBuffer = beginSingleTimeCommands(context);

    VkBufferCopy copyRegion={};
    copyRegion.size =size;
    vkCmdCopyBuffer(commandBuffer,srcBuffer,dstBuffer,1,&copyRegion);

    endSingleTimeCommands(context,commandBuffer);
}

std::vector<char> VulkanUtils::readFile(const std::string& filename){
    std::ifstream file(filename,std::ios::ate | std::ios::binary);
    if(!file.is_open()){
        throw std::runtime_error("open file failed: "+ filename);
    }

    size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(),(long long)file_size);
    file.close();
    return buffer;
}


VkCommandBuffer VulkanUtils::beginSingleTimeCommands(const VulkanContext* context) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool =context->commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context->device, &allocInfo, &commandBuffer);
    assert(commandBuffer !=VK_NULL_HANDLE);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}


void VulkanUtils::createImage2D(const VulkanContext *context,
                                uint32_t width, uint32_t height,  uint32_t mipLevel, VkSampleCountFlagBits numberSample,
                                VkFormat format,VkImageTiling tiling,
                                VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                                VkImage &image, VkDeviceMemory &memory) {
    //Create  Buffer
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevel;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numberSample;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    VK_CHECK(vkCreateImage(context->device, &imageInfo, nullptr, &image),
             "failed to create textures!");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(context->device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(context->physicalDevice,memRequirements.memoryTypeBits, properties);
    VK_CHECK(vkAllocateMemory(context->device, &allocInfo, nullptr, &memory) ,"failed to allocate vertex buffer memory!");
    VK_CHECK(vkBindImageMemory(context->device, image, memory, 0),"Bind Buffer VertexBuffer Failed");
}


void VulkanUtils::createCubeImage(const VulkanContext *context, uint32_t width, uint32_t height,
                                  uint32_t mipLevel, VkSampleCountFlagBits numberSample, VkFormat format,
                                  VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                                  VkImage &image, VkDeviceMemory &memory) {
    //Create  Buffer
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevel;
    imageInfo.arrayLayers = 6;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numberSample;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VK_CHECK(vkCreateImage(context->device, &imageInfo, nullptr, &image),
             "failed to create textures!");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(context->device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(context->physicalDevice,memRequirements.memoryTypeBits, properties);
    VK_CHECK(vkAllocateMemory(context->device, &allocInfo, nullptr, &memory) ,"failed to allocate image buffer memory!");
    VK_CHECK(vkBindImageMemory(context->device, image, memory, 0),"Bind Buffer  Failed");

}


VkImageView VulkanUtils::createImageCubeView(VkDevice device,
                                        VkImage image,
                                        VkFormat format,
                                        VkImageAspectFlags aspectFlags,
                                        uint32_t baseMipLayer,
                                        uint32_t numMipLevel)
{
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
    VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &textureImageView),"failed to create texture image view!");
    return textureImageView;
}

VkImageView VulkanUtils::createImageView(VkDevice device,
                                     VkImage image,
                                     VkFormat format,
                                     VkImageAspectFlags aspectFlags,
                                     VkImageViewType viewType,
                                     uint32_t baseMipLayer,
                                     uint32_t numMipLevel ,
                                     uint32_t baseLayer,
                                     uint32_t numLayers)
{
    VkImageView textureImageView;
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = viewType;
    viewInfo.format = format;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = baseMipLayer;
    viewInfo.subresourceRange.levelCount = numMipLevel;
    viewInfo.subresourceRange.baseArrayLayer = baseLayer;
    viewInfo.subresourceRange.layerCount = numLayers;

    VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &textureImageView),"failed to create texture image view!");
    return textureImageView;
}


void VulkanUtils::transitionImageLayout(const VulkanContext* context,
                                   VkImage image,
                                   VkFormat format,
                                   VkImageLayout oldLayout,
                                   VkImageLayout newLayout,
                                   uint32_t baseMipLayer,
                                   uint32_t numMipLevel,
                                   uint32_t baseLayer ,
                                   uint32_t numLayers )
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.baseMipLevel =baseMipLayer;
    barrier.subresourceRange.levelCount = numMipLevel;
    barrier.subresourceRange.baseArrayLayer = baseLayer;
    barrier.subresourceRange.layerCount = numLayers;

    if(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
            srcStage , dstStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
    );
    endSingleTimeCommands(context,commandBuffer);
}


void VulkanUtils::copyBufferToImage(const VulkanContext *context, VkBuffer srcBuffer, VkImage dstBuffer, uint32_t width,
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
    region.imageExtent = {
            width,
            height,
            1
    };
    vkCmdCopyBufferToImage(
            commandBuffer,
            srcBuffer,
            dstBuffer,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
    );
    endSingleTimeCommands(context,commandBuffer);
}



VkSampler VulkanUtils::createSampler(VkDevice device,
                                       uint32_t mipLevels){
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
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod =static_cast<float>(mipLevels);
    VkSampler textureSampler{VK_NULL_HANDLE};
    VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler),"failed to create texture sampler!") ;

    return textureSampler;
}


bool VulkanUtils::hasStencilComponent(VkFormat format){
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void
VulkanUtils::generateImage2DMipMaps(const VulkanContext *context,
                                    VkImage image,uint32_t width, uint32_t height,
                                    uint32_t mipLevel,VkFormat format,
                                    VkFilter filter) {
    if(mipLevel == 1){
        return ;
    }
    VkFormatProperties formatProperties;
    bool supportsLinearFiltering = (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0;
    bool supportCubicFiltering = (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT) == 0;

    //TODO Fix
    vkGetPhysicalDeviceFormatProperties(context->physicalDevice, format, &formatProperties);
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

    int mipWidth =width;
    int mipHeight = height;

    for (uint32_t i = 1; i <  mipLevel; i++) {
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
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
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

        mipWidth = std::max(1,mipWidth/=2);
        mipHeight= std::max(1,mipHeight/=2);
    }

    endSingleTimeCommands(context,commandBuffer);

}


VkSampleCountFlagBits VulkanUtils::getMaxUsableSampleCount(VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = std::min(
            physicalDeviceProperties.limits.framebufferColorSampleCounts,
            physicalDeviceProperties.limits.framebufferDepthSampleCounts
    );
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

VkShaderModule VulkanUtils::createShaderModule(VkDevice device,
                                         uint32_t * code,
                                         uint32_t size){
    VkShaderModuleCreateInfo shaderInfo={};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.pCode =code;
    shaderInfo.codeSize =size;
    VkShaderModule shader;
    VK_CHECK(vkCreateShaderModule(device,&shaderInfo, nullptr,&shader),"Create shader module failed\n");
    return shader;
}


void VulkanUtils::bindUniformBuffer(VkDevice device,
        VkDescriptorSet descriptorSet,
        int binding,
        VkBuffer buffer,
        VkDeviceSize offset,
        VkDeviceSize size
)
{
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
)
{
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

void VulkanUtils::endSingleTimeCommands(const VulkanContext* context,VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;

    VkFence fence{};
    VK_CHECK(vkCreateFence(context->device,&fenceInfo, nullptr,&fence),"Create Fence Failed\n");

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    VK_CHECK( vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, fence),"failed to submit command buffer");
    VK_CHECK(vkWaitForFences(context->device,1,&fence,VK_TRUE,10000000000),"Wait for Fence failed");
    vkDestroyFence(context->device,fence, nullptr);
    vkFreeCommandBuffers(context->device, context->commandPool, 1, &commandBuffer);
}


VkFormat VulkanUtils::selectOptimalSupportedImageFormat(const VkPhysicalDevice& physicalDevice,const std::vector<VkFormat> &candiates,
                                                            VkImageTiling tiling, VkFormatFeatureFlags features) {
    for(VkFormat format : candiates)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice,format,&properties);
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
            return format;

        if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
        std::cout << format<<"\n";
    }

    TH_WITH_MSG(true,"can not find support format");
}

VkFormat  VulkanUtils::selectOptimalImageFormat(const VkPhysicalDevice& physicalDevice) {
    return selectOptimalSupportedImageFormat(physicalDevice,
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

/*
 */
bool VulkanUtils::checkInstanceValidationLayers(
        const std::vector<const char *> &requiredLayers,
        bool verbose
)
{
    uint32_t availableLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(availableLayerCount);
    vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());

    for (const char *requiredLayer : requiredLayers)
    {
        bool supported = false;
        for (const VkLayerProperties &layer : availableLayers)
        {
            if (strcmp(requiredLayer, layer.layerName) == 0)
            {
                supported = true;
                break;
            }
        }

        if (!supported)
        {
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
)
{
    uint32_t availableExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

    for (const char *requiredExtension : requiredExtensions)
    {
        bool supported = false;
        for (const VkExtensionProperties &availableExtension : availableExtensions)
        {
            if (strcmp(requiredExtension, availableExtension.extensionName) == 0)
            {
                supported = true;
                break;
            }
        }

        if (!supported)
        {
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
)
{
    uint32_t availableDeviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableDeviceExtensionCount, nullptr);

    std::vector<VkExtensionProperties> availableDeviceExtensions(availableDeviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableDeviceExtensionCount, availableDeviceExtensions.data());

    for (const char *requiredExtension : requiredExtensions)
    {
        bool supported = false;
        for (const VkExtensionProperties &availableDeviceExtension : availableDeviceExtensions)
        {
            if (strcmp(requiredExtension, availableDeviceExtension.extensionName) == 0)
            {
                supported = true;
                break;
            }
        }

        if (!supported)
        {
            if (verbose)
                std::cerr << requiredExtension << " is not supported on this physical device" << std::endl;

            return false;
        }

        if (verbose)
            std::cout << "Have " << requiredExtension << std::endl;
    }

    return true;
}
