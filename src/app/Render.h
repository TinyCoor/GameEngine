//
// Created by y123456 on 2021/10/10.
//

#ifndef GAMEENGINE_RENDER_H
#define GAMEENGINE_RENDER_H
#include "macro.h"
#include "render_data.h"
#include "vertex.h"
#include <vulkan.h>
#include <string>
#include <vector>
#include <stdexcept>

class Render{
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
    explicit Render(RenderContext& ctx,RenderData& renderData)
                    :context(ctx),data(renderData){
    }

    void init(const std::string& vertShaderFile,
              const std::string& fragShaderFile);

    VkCommandBuffer render(uint32_t imageIndex);
    void shutdown();

};


#endif //GAMEENGINE_RENDER_H
