//
// Created by y123456 on 2021/10/13.
//

#include "vulkan_utils.h"
#include "macro.h"
#include <stdexcept>
#include <fstream>

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
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = context.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context.device_, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer,&beginInfo);

    VkBufferCopy copyRegion={};
    copyRegion.size =size;
    vkCmdCopyBuffer(commandBuffer,srcBuffer,dstBuffer,1,&copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(context.graphicsQueue);

    vkFreeCommandBuffers(context.device_,context.commandPool,1,&commandBuffer);


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

