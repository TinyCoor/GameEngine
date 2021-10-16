//
// Created by y123456 on 2021/10/10.
//

#ifndef GAMEENGINE_VULKANRENDER_H
#define GAMEENGINE_VULKANRENDER_H
#include "Macro.h"
#include "VulkanRenderData.h"
#include "VulkanRenderContext.h"
#include <vulkan.h>
#include <string>
#include <vector>
#include <stdexcept>

class VulkanRender{
private:
    RenderData data;
    RenderContext context;
    VkRenderPass renderPass =VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout=VK_NULL_HANDLE;
    VkPipeline graphicsPipeLine = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptorSetLayout =VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets{};

    std::vector<VkFramebuffer> frameBuffers{};
    std::vector<VkCommandBuffer> commandBuffers{};

    std::vector<VkBuffer> uniformBuffers{};
    std::vector<VkDeviceMemory> uniformBuffersMemory{};

public:
    explicit VulkanRender(RenderContext& ctx,RenderData& renderData)
                    :context(ctx),data(renderData){
    }

    void init(const std::string& vertShaderFile,
              const std::string& fragShaderFile,
              const std::string& textureFile,
              const std::string& model_path);

    VkCommandBuffer render(uint32_t imageIndex);
    void shutdown();
};


#endif //GAMEENGINE_VULKANRENDER_H
