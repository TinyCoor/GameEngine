//
// Created by y123456 on 2021/10/10.
//

#ifndef GAMEENGINE_VULKANRENDER_H
#define GAMEENGINE_VULKANRENDER_H

#include "VulkanRenderScene.h"
#include "VulkanRenderContext.h"
#include <string>
#include <vector>
#include <stdexcept>

class VulkanCubeMapRender {
private:
    VulkanRenderContext context;

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
    VulkanCubeMapRender(VulkanRenderContext& ctx, VulkanSwapChainContext& swapChainCtx)
                    :context(ctx){}

    void init(VkExtent2D extent);
    void shutdown();

    VkCommandBuffer render();
};


#endif //GAMEENGINE_VULKANRENDER_H
