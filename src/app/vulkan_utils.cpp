//
// Created by y123456 on 2021/10/13.
//

#include "vulkan_utils.h"
#include "macro.h"
#include <stdexcept>
#include <fstream>
#include <stb_image.h>

void vulkanUtils::createBuffer(const RenderContext& context,
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
    VK_CHECK(vkCreateBuffer(context.device_, &bufferInfo, nullptr, &buffer),
             "failed to create vertex buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.device_, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(context,memRequirements.memoryTypeBits, memoryFlags);
    VK_CHECK(vkAllocateMemory(context.device_, &allocInfo, nullptr, &memory) ,"failed to allocate vertex buffer memory!");
    VK_CHECK( vkBindBufferMemory(context.device_, buffer, memory, 0),"Bind Buffer VertexBuffer Failed");

}



uint32_t vulkanUtils::findMemoryType(const RenderContext& context,
                                     uint32_t typeFilter,
                                     VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(context.physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        uint32_t memoryPropertiy   = memProperties.memoryTypes[i].propertyFlags;
        if (typeFilter & (1 << i) && (memoryPropertiy & properties) == properties){
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

void vulkanUtils::copyBuffer(const RenderContext& context,
                             VkBuffer srcBuffer,
                             VkBuffer dstBuffer,
                             VkDeviceSize size) {


    auto commandBuffer = beginSingleTimeCommands(context);

    VkBufferCopy copyRegion={};
    copyRegion.size =size;
    vkCmdCopyBuffer(commandBuffer,srcBuffer,dstBuffer,1,&copyRegion);

    endSingleTimeCommands(context,commandBuffer);
}

std::vector<char> vulkanUtils::readFile(const std::string& filename){
    std::ifstream file(filename,std::ios::ate | std::ios::binary);
    if(!file.is_open()){
        throw std::runtime_error("open file failed");
    }

    size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(),file_size);
    file.close();
    return buffer;
}

void vulkanUtils::endSingleTimeCommands(const RenderContext& context,VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(context.graphicsQueue);

    vkFreeCommandBuffers(context.device_, context.commandPool, 1, &commandBuffer);
}

VkCommandBuffer vulkanUtils::beginSingleTimeCommands(const RenderContext& context) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool =context.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context.device_, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}


void vulkanUtils::createImage2D(const RenderContext &context, uint32_t width, uint32_t height, VkFormat format,
                                VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                                VkImage &image, VkDeviceMemory &memory) {
    //Create Vertex Buffer
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateImage(context.device_, &imageInfo, nullptr, &image),
             "failed to create textures!");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(context.device_, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(context,memRequirements.memoryTypeBits, usage);
    VK_CHECK(vkAllocateMemory(context.device_, &allocInfo, nullptr, &memory) ,"failed to allocate vertex buffer memory!");
    VK_CHECK(vkBindImageMemory(context.device_, image, memory, 0),"Bind Buffer VertexBuffer Failed");
}

void
vulkanUtils::transitionImageLayout(const RenderContext& context,
                                   VkImage image,
                                   VkFormat format,
                                   VkImageLayout oldLayout,
                                   VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

//    barrier.srcAccessMask = 0; // TODO
//    barrier.dstAccessMask = 0; // TODO
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }


    vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage /* TODO */, destinationStage /* TODO */,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
    );

    endSingleTimeCommands(context,commandBuffer);

}

void vulkanUtils::copyBufferToImage(const RenderContext &context, VkBuffer srcBuffer, VkImage dstBuffer, uint32_t width,
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
