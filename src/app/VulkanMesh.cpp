//
// Created by y123456 on 2021/10/16.
//

#include "VulkanMesh.h"
#include "VulkanUtils.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>


VulkanMesh::~VulkanMesh(){
//    clearGPUData();
//    clearCPUData();
}

VkVertexInputBindingDescription VulkanMesh::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> VulkanMesh:: getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions ={};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, uv);

    return attributeDescriptions;
}


void VulkanMesh::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(Vertex) *  vertices.size();
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

void VulkanMesh::createIndexBuffer() {
    //Create Index Buffer
    VkDeviceSize bufferSize = sizeof (uint32_t) * indices.size();

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
    memcpy(data,indices.data(), bufferSize);
    vkUnmapMemory(context.device_, stagingBufferMemory);

    vulkanUtils::copyBuffer(context,stagingBuffer,indexBuffer,bufferSize);

    vkDestroyBuffer(context.device_,stagingBuffer, nullptr);
    vkFreeMemory(context.device_, stagingBufferMemory, nullptr);
}

bool VulkanMesh::loadFromFile(const std::string &file) {
    Assimp::Importer importer;
    unsigned int flags =aiProcess_Triangulate | aiProcess_FlipUVs;
    const aiScene* scene = importer.ReadFile(file,flags);
    if(!scene){
        std::cout << "Load Model failed:"<<file << "Error: "<<importer.GetErrorString();
        return false;
    }

    if(!scene->HasMeshes()) {
        std::cerr<< "No mesh In the file\n";
        return false;
    }else{
        aiMesh* mesh = scene->mMeshes[0];
        assert(mesh !=nullptr);
        vertices.resize(mesh->mNumVertices);
        Vertex vertex;
        for(unsigned int i =0; i < mesh->mNumVertices;++i){
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;
            if(mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.uv = vec;
            }else
                vertex.uv = glm::vec2(0.0f, 0.0f);
            vertices.push_back(vertex);
        }

        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
    }
    // upload to  GPU
    uploadToGPU();
    //TODO Clear GPU Data
    return true;
}

void VulkanMesh::clearGPUData() {
    vkDestroyBuffer(context.device_,vertexBuffer, nullptr);
    vkFreeMemory(context.device_,vertexBufferMemory, nullptr);
    vkDestroyBuffer(context.device_,indexBuffer, nullptr);
    vkFreeMemory(context.device_,indexBufferMemory, nullptr);
    vertexBuffer =VK_NULL_HANDLE;
    vertexBufferMemory = VK_NULL_HANDLE;
}

void VulkanMesh::uploadToGPU() {
    if(!vertices.empty()){
        createVertexBuffer();
    }
    if(!indices.empty()){
        createIndexBuffer();
    }


}

void VulkanMesh::clearCPUData() {
    vertices.clear();
    indices.clear();
}
