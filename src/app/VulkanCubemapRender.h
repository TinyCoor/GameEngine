//
// Created by y123456 on 2021/10/10.
//

#pragma once

#include "VulkanRenderScene.h"
#include "VulkanRenderContext.h"
#include <string>
#include <vector>
#include <stdexcept>

class VulkanCubeMapRender {
private:
    VulkanRenderContext context;
    VulkanShader renderFragmentShader;
    VulkanShader renderVertexShader;
    VulkanMesh renderQuad;

    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    VkPipeline pbrPipeline{VK_NULL_HANDLE};
    VkPipelineLayout  pbrPipelineLayout{VK_NULL_HANDLE};

    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VkFramebuffer frameBuffer{VK_NULL_HANDLE};
    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};

    VkBuffer uniformBuffer{VK_NULL_HANDLE};
    VkDeviceMemory uniformBuffersMemory{VK_NULL_HANDLE};



public:
    VulkanCubeMapRender(VulkanRenderContext& ctx)
                    :context(ctx), renderFragmentShader(ctx),
                      renderQuad(ctx), renderVertexShader(ctx){}

    void init(const VulkanTexture& inputTexture,const VulkanTexture& targetTexture);
    void shutdown();

    void render();
};
