//
// Created by y123456 on 2021/10/12.
//

#include <stdexcept>
#include "render_data.h"
#include "macro.h"

void RenderData::init(const std::string& vertShaderFile,
                      const std::string& fragShaderFile){
    vertShader = createShader(vertShaderFile);
    fragShader = createShader(fragShaderFile);

   createVertexBuffer();
   createIndexBuffer();
}

void RenderData::shutdown(){
    vkDestroyShaderModule(context.device_,vertShader,nullptr);
    vkDestroyShaderModule(context.device_,fragShader,nullptr);
    vkDestroyBuffer(context.device_,vertexBuffer, nullptr);
    vkFreeMemory(context.device_,vertexBufferMemory, nullptr);
    vkDestroyBuffer(context.device_,indexBuffer, nullptr);
    vkFreeMemory(context.device_,indexBufferMemory, nullptr);
    vertShader = VK_NULL_HANDLE;
    fragShader = VK_NULL_HANDLE;
    vertexBuffer =VK_NULL_HANDLE;
    vertexBufferMemory = VK_NULL_HANDLE;
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







void RenderData::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(Vertex)* vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    vulkanUtils::createBuffer(context,bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 vertexBuffer,
                 vertexBufferMemory);


    vulkanUtils::createBuffer(context,bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    void* data = nullptr;
    vkMapMemory(context.device_, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), bufferSize);
    vkUnmapMemory(context.device_, stagingBufferMemory);

    vulkanUtils::copyBuffer(context,stagingBuffer,vertexBuffer,bufferSize);

    vkDestroyBuffer(context.device_,stagingBuffer, nullptr);
    vkFreeMemory(context.device_, stagingBufferMemory, nullptr);
}

void RenderData::createIndexBuffer() {
    //Create Index Buffer
    VkDeviceSize bufferSize = sizeof(uint16_t) *indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    vulkanUtils::createBuffer(context,bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 indexBuffer,
                 indexBufferMemory);

    vulkanUtils:: createBuffer(context,bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    void* data = nullptr;
    vkMapMemory(context.device_, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), bufferSize);
    vkUnmapMemory(context.device_, stagingBufferMemory);

    vulkanUtils::copyBuffer(context,stagingBuffer,indexBuffer,bufferSize);

    vkDestroyBuffer(context.device_,stagingBuffer, nullptr);
    vkFreeMemory(context.device_, stagingBufferMemory, nullptr);
}