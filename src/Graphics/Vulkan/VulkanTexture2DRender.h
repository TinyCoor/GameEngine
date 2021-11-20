//
// Created by 12132 on 2021/11/9.
//

#ifndef GAMEENGINE_VULKANTEXTURE2DRENDER_H
#define GAMEENGINE_VULKANTEXTURE2DRENDER_H
#include <memory>
#include <volk.h>

class VulkanContext;
class VulkanMesh;
class VulkanTexture;
class VulkanShader;


class VulkanTexture2DRender {
public:
    VulkanTexture2DRender(const VulkanContext* ctx);

    void init(std::shared_ptr <VulkanShader> vertShader,
              std::shared_ptr <VulkanShader> fragShader,
              std::shared_ptr <VulkanTexture> targetTexture);

    void shutdown();

    void render();

private:
    const VulkanContext* context;
    std::shared_ptr<VulkanMesh> renderQuad;
    VkExtent2D targetExtent;

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;

    VkFramebuffer framebuffer;
    VkCommandBuffer commandBuffer;
    VkFence fence;
};


#endif //GAMEENGINE_VULKANTEXTURE2DRENDER_H
