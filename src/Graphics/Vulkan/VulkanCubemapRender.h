//
// Created by y123456 on 2021/10/10.
//

#pragma once

#include "VulkanContext.h"
#include <memory>

class VulkanMesh;
class VulkanShader;
class VulkanTexture;

class VulkanCubeMapRender {
private:
    const VulkanContext* context;
    std::shared_ptr<VulkanMesh> renderQuad;

    VkExtent2D targetExtent;

    VkImageView faceViews[6];
    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    VkPipeline pipeline{VK_NULL_HANDLE};
    VkPipelineLayout  pipelineLayout{VK_NULL_HANDLE};

    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VkFramebuffer frameBuffer{VK_NULL_HANDLE};

    VkBuffer uniformBuffer{VK_NULL_HANDLE};
    VkDeviceMemory uniformBuffersMemory{VK_NULL_HANDLE};

    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};

    VkFence fence {VK_NULL_HANDLE};

    uint32_t pushConstantsSize {0};


public:
    VulkanCubeMapRender(const VulkanContext* ctx);

    void init(std::shared_ptr <VulkanShader> vertShader,
              std::shared_ptr <VulkanShader> fragShader,
              std::shared_ptr <VulkanTexture> targetTexture,
              int mip,
              uint32_t userDataSize = 0);

    void shutdown();

    void render(std::shared_ptr <VulkanTexture> inputTexture,
               float * userData = nullptr, int mip= -1);
};
