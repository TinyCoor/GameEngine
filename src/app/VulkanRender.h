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


class VulkanRender {
private:
    VulkanRenderContext context;
    VulkanSwapChainContext swapChainContext;

    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};


    VkPipeline pbrPipeline{VK_NULL_HANDLE};
    VkPipelineLayout  pbrPipelineLayout{VK_NULL_HANDLE};

    VkPipeline skyboxPipeline{VK_NULL_HANDLE};
    VkPipelineLayout   skyboxPipelineLayout{VK_NULL_HANDLE};



    std::vector<VkCommandBuffer> commandBuffers{};
    std::vector<VkFramebuffer> frameBuffers{};

    std::vector<VkBuffer> uniformBuffers{};
    std::vector<VkDeviceMemory> uniformBuffersMemory{};

    std::vector<VkDescriptorSet> descriptorSets{};

public:
    explicit VulkanRender(VulkanRenderContext& ctx, VulkanSwapChainContext& swapChainCtx)
                    :context(ctx),swapChainContext(swapChainCtx){
    }

    void init(VulkanRenderScene* scene);

    VkCommandBuffer render(uint32_t imageIndex);
    void shutdown();
};


#endif //GAMEENGINE_VULKANRENDER_H
