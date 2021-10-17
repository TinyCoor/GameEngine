//
// Created by y123456 on 2021/10/16.
//

#ifndef GAMEENGINE_VULKANRENDERMODEL_H
#define GAMEENGINE_VULKANRENDERMODEL_H
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <array>
#include <vector>
#include "VulkanRenderContext.h"

struct UniformBufferObject{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class  VulkanMesh{
private:
    struct Vertex{
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 uv;
    };
    VulkanRenderContext context;
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    };

    std::vector<uint32_t> indices= {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
    };

    VkBuffer vertexBuffer =VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory =VK_NULL_HANDLE;

    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory= VK_NULL_HANDLE;

public:
    VulkanMesh(const VulkanRenderContext& ctx): context(ctx){}
    ~VulkanMesh();
    inline uint32_t getNumIndices() const {return indices.size();}
    inline VkBuffer getVertexBuffer() const { return vertexBuffer;}
    inline VkBuffer getIndexBuffer() const { return indexBuffer;}

    static VkVertexInputBindingDescription getBindingDescription() ;
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() ;

    bool loadFromFile(const std::string& file);
    void uploadToGPU();
    void clearGPUData();
    void clearCPUData();
private:
    void createIndexBuffer();
    void createVertexBuffer();
};

#endif //GAMEENGINE_VULKANRENDERMODEL_H
