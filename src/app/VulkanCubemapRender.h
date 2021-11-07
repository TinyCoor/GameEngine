//
// Created by y123456 on 2021/10/10.
//

#pragma once


#include "VulkanRenderContext.h"

#include <memory>

class VulkanMesh;
class VulkanTexture;
class VulkanShader;

class VulkanCubeMapRender {
private:
    VulkanRenderContext context;
    std::shared_ptr<VulkanMesh> renderQuad;
    VkImageView faceViews[6];
    VkExtent2D targetExtent;


    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    VkPipeline pipeline{VK_NULL_HANDLE};
    VkPipelineLayout  pipelineLayout{VK_NULL_HANDLE};

    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VkFramebuffer frameBuffer{VK_NULL_HANDLE};
    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};


    VkFence fence {VK_NULL_HANDLE};

    VkBuffer uniformBuffer{VK_NULL_HANDLE};
    VkDeviceMemory uniformBuffersMemory{VK_NULL_HANDLE};



public:
    VulkanCubeMapRender(VulkanRenderContext& ctx);

    void init(std::shared_ptr <VulkanShader> vertShader,
              std::shared_ptr <VulkanShader> fragShader,
              std::shared_ptr <VulkanTexture> targetTexture);

    void shutdown();

    void render(std::shared_ptr <VulkanTexture> inputTexture);
};
