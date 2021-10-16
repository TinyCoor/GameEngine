//
// Created by y123456 on 2021/10/16.
//

#include "VulkanMesh.h"
#include "VulkanUtils.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


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

//This is a bug in load form File
bool VulkanMesh::loadFromFile(const std::string &file) {
    clearCPUData();
#ifdef true
    Assimp::Importer importer;
    unsigned int flags = aiProcess_Triangulate;
    const aiScene* scene = importer.ReadFile(file,flags);
    if(!scene){
        std::cout << "Load Model failed:"<<file << "Error: "<<importer.GetErrorString();
        return false;
    }if(!scene->HasMeshes()) {
        std::cerr<< "No mesh In the file\n";
        return false;
    }else{
        aiMesh* mesh = scene->mMeshes[0];
        assert(mesh != nullptr);

        vertices.resize(mesh->mNumVertices);
        indices.resize(mesh->mNumFaces * 3);

        aiVector3D* vertex =mesh-> mVertices;
        for(unsigned int i=0;i< mesh->mNumVertices;i++)
            vertices[i].position= glm::vec3(vertex[i].x,vertex[i].y,vertex[i].z);

        aiColor4D* colors = mesh->mColors[0];
        if(colors){
            for (unsigned int i = 0; i < mesh->mNumVertices ; ++i)
                vertices[i].color= glm::vec3 (colors[i].r,colors[i].g,colors[i].b);
        } else{
            for (unsigned int i = 0; i <  mesh->mNumVertices ; ++i)
                vertices[i].color= glm::vec3 (1.0,1.0,1.0);
        }

        aiFace* faces =mesh->mFaces;
        unsigned int index =0;
        for (unsigned int i = 0; i <mesh->mNumFaces; ++i) {
            for (int faceIndex = 0; faceIndex <faces[i].mNumIndices; ++faceIndex) {
                indices[index++] = faces[i].mIndices[faceIndex];
            }
        }
        // uv
        aiVector3D* uvs = mesh->mTextureCoords[0];
        if(uvs){
            for (unsigned int i = 0; i < mesh->mNumVertices ; ++i) {
                vertices[i].uv = glm::vec2(uvs[i].x,1.0-uvs[i].y);
            }
        }else{
            for (unsigned int i = 0; i < mesh->mNumVertices ; ++i) {
                vertices[i].uv = glm::vec2(0.0,0.0);
            }
        }
    }
#else
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file.c_str())) {
        throw std::runtime_error(warn + err);
    }
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };
            vertex.color = {1.0f, 1.0f, 1.0f};
            vertices.push_back(vertex);
            indices.push_back(indices.size());
        }
    }
#endif

    // upload to  GPU
    clearGPUData();
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
    indexBuffer= VK_NULL_HANDLE;
    indexBufferMemory=VK_NULL_HANDLE;
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
