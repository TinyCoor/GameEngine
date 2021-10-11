//
// Created by y123456 on 2021/10/10.
//

#ifndef GAMEENGINE_RENDER_H
#define GAMEENGINE_RENDER_H

#include <vulkan.h>
#include <string>
#include <vector>
struct QueueFamilyIndices{
    std::pair<bool,uint32_t> graphicsFamily;
    std::pair<bool,uint32_t> presentFamily{std::make_pair(false,0)};

    bool isComplete()const{
        return graphicsFamily.first && presentFamily.first;
    }
};

struct RenderContext{
    VkDevice device_ =VK_NULL_HANDLE;
    VkExtent2D extend;
    VkFormat format ;
    QueueFamilyIndices queueFamilyIndex;
    std::vector<VkImageView> imageViews;
};

class Render{
private:
    RenderContext context;


    VkRenderPass renderPass =VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout=VK_NULL_HANDLE;
    VkPipeline graphicsPipeLine = VK_NULL_HANDLE;
    VkCommandPool commandPool =VK_NULL_HANDLE;
    VkShaderModule vertShader=VK_NULL_HANDLE;
    VkShaderModule fragShader=VK_NULL_HANDLE;
    std::vector<VkFramebuffer> frameBuffers;
    std::vector<VkCommandBuffer> commandBuffers;
private:
    VkShaderModule createShader(const std::string& path) const;

public:
    explicit Render(RenderContext& ctx)
                    :context(ctx){
    }

    void init(const std::string& vertShaderFile,const std::string& fragShaderFile);
    VkCommandBuffer render(uint32_t index);
    void shutdown();

};


#endif //GAMEENGINE_RENDER_H
