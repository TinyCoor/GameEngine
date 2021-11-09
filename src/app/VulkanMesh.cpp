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
    clearGPUData();
    clearCPUData();
}

VkVertexInputBindingDescription VulkanMesh::getVertexInputBindingDescription() {
   static  VkVertexInputBindingDescription bindingDescription={
            .binding =0,
            .stride = sizeof(Vertex),
            .inputRate =VK_VERTEX_INPUT_RATE_VERTEX
    };
    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> VulkanMesh:: getAttributeDescriptions() {
   static  std::vector<VkVertexInputAttributeDescription> attributes{
           { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) },
           { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent) },
           { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, binormal) },
           { 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) },
           { 4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) },
           { 5, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) },
   };

    return attributes;
}


void VulkanMesh::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(Vertex) *  vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VulkanUtils::createBuffer(context,bufferSize,
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              vertexBuffer,
                              vertexBufferMemory);


    VulkanUtils::createBuffer(context,bufferSize,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              stagingBuffer,
                              stagingBufferMemory);

    void* data = nullptr;
    vkMapMemory(context->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), bufferSize);
    vkUnmapMemory(context->device, stagingBufferMemory);

    VulkanUtils::copyBuffer(context,stagingBuffer,vertexBuffer,bufferSize);

    vkDestroyBuffer(context->device,stagingBuffer, nullptr);
    vkFreeMemory(context->device, stagingBufferMemory, nullptr);
}

void VulkanMesh::createIndexBuffer() {
    //Create Index Buffer
    VkDeviceSize bufferSize = sizeof (uint32_t) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VulkanUtils::createBuffer(context,bufferSize,
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              indexBuffer,
                              indexBufferMemory);

    VulkanUtils:: createBuffer(context,bufferSize,
                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               stagingBuffer,
                               stagingBufferMemory);

    void* data = nullptr;
    vkMapMemory(context->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data,indices.data(), bufferSize);
    vkUnmapMemory(context->device, stagingBufferMemory);

    VulkanUtils::copyBuffer(context,stagingBuffer,indexBuffer,bufferSize);

    vkDestroyBuffer(context->device,stagingBuffer, nullptr);
    vkFreeMemory(context->device, stagingBufferMemory, nullptr);
}

//This is a bug in load form File
bool VulkanMesh::loadFromFile(const char* file) {
    clearCPUData();
    clearGPUData();
#ifndef false
    Assimp::Importer importer;
    unsigned int flags = aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType;
    const aiScene* scene = importer.ReadFile(file,flags);
    if(!scene){
        std::cout << "Load Model failed:"<<file << "Error: "<<importer.GetErrorString();
        return false;
    }if(!scene->HasMeshes()) {
        std::cerr<< "No mesh In the file\n";
        return false;
    }else{
        aiMesh *mesh = scene->mMeshes[0];
        assert(mesh != nullptr);

        // Fill CPU data
        vertices.resize(mesh->mNumVertices);
        indices.resize(mesh->mNumFaces * 3);

        aiVector3D *meshVertices = mesh->mVertices;
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            vertices[i].position = glm::vec3(meshVertices[i].x, meshVertices[i].y, meshVertices[i].z);

        aiVector3D *meshTangents = mesh->mTangents;
        if (meshTangents)
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                vertices[i].tangent = glm::vec3(meshTangents[i].x, meshTangents[i].y, meshTangents[i].z);
        else
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                vertices[i].tangent = glm::vec3(0.0f, 0.0f, 0.0f);

        aiVector3D *meshBinormals = mesh->mBitangents;
        if (meshBinormals)
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                vertices[i].binormal = glm::vec3(meshBinormals[i].x, meshBinormals[i].y, meshBinormals[i].z);
        else
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                vertices[i].binormal = glm::vec3(0.0f, 0.0f, 0.0f);

        aiVector3D *meshNormals = mesh->mNormals;
        if (meshNormals)
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                vertices[i].normal = glm::vec3(meshNormals[i].x, meshNormals[i].y, meshNormals[i].z);
        else
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                vertices[i].normal = glm::vec3(0.0f, 0.0f, 0.0f);

        aiVector3D *meshUVs = mesh->mTextureCoords[0];
        if (meshUVs)
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                vertices[i].uv = glm::vec2(meshUVs[i].x, 1.0f - meshUVs[i].y);
        else
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                vertices[i].uv = glm::vec2(0.0f, 0.0f);

        aiColor4D *meshColors = mesh->mColors[0];
        if (meshColors)
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                vertices[i].color = glm::vec3(meshColors[i].r, meshColors[i].g, meshColors[i].b);
        else
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                vertices[i].color = glm::vec3(1.0f, 1.0f, 1.0f);

        aiFace *meshFaces = mesh->mFaces;
        unsigned int index = 0;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            for (unsigned int faceIndex = 0; faceIndex < meshFaces[i].mNumIndices; faceIndex++)
                indices[index++] = meshFaces[i].mIndices[faceIndex];

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
    vkDestroyBuffer(context->device,vertexBuffer, nullptr);
    vkFreeMemory(context->device,vertexBufferMemory, nullptr);
    vkDestroyBuffer(context->device,indexBuffer, nullptr);
    vkFreeMemory(context->device,indexBufferMemory, nullptr);
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


void VulkanMesh::createSkybox(float size) {
    size *=0.5;
    vertices.resize(8);
    indices.resize(36);
    vertices[0].position = glm::vec3 (-size ,-size,-size);
    vertices[1].position = glm::vec3 ( size ,-size,-size);
    vertices[2].position = glm::vec3 ( size , size,-size);
    vertices[3].position = glm::vec3 (-size , size,-size);

    vertices[4].position = glm::vec3 (-size ,-size, size);
    vertices[5].position = glm::vec3 ( size ,-size, size);
    vertices[6].position = glm::vec3 ( size , size, size);
    vertices[7].position = glm::vec3 (-size , size, size);

    indices={
        0,  1,  2,  2,  3, 0,
        1,  5,  6,  6,  2, 1,
        3,  2,  6,  6,  7, 3,
        5,  4,  6,  4,  7, 6,
        1,  0,  4,  4,  5, 1,
        4,  0,  3,  3,  7, 4,
    };

    uploadToGPU();
}

void VulkanMesh::createQuad(float size) {

    float halfSize = size * 0.5f;
    vertices.resize(4);
    indices.resize(6);

    vertices[0].position = glm::vec3(-halfSize, -halfSize, 0.0f);
    vertices[1].position = glm::vec3( halfSize, -halfSize, 0.0f);
    vertices[2].position = glm::vec3( halfSize,  halfSize, 0.0f);
    vertices[3].position = glm::vec3(-halfSize,  halfSize, 0.0f);

    vertices[0].uv = glm::vec2(0.0f, 0.0f);
    vertices[1].uv = glm::vec2(1.0f, 0.0f);
    vertices[2].uv = glm::vec2(1.0f, 1.0f);
    vertices[3].uv = glm::vec2(0.0f, 1.0f);

    indices = {
            1, 0, 2, 3, 2, 0,
    };

    uploadToGPU();

}
