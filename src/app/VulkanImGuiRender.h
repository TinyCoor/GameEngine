//
// Created by 12132 on 2021/11/7.
//

#ifndef GAMEENGINE_VULKANIMGUIRENDER_H
#define GAMEENGINE_VULKANIMGUIRENDER_H
#include "VulkanRenderContext.h"
#include <memory>
struct RenderState;
class VulkanRenderScene;
struct VulkanRenderFrame;
class VulkanSwapChain;

class VulkanImGuiRender {
public:
    explicit VulkanImGuiRender(const VulkanRenderContext& ctx,
                               VkExtent2D extent,
                               VkRenderPass renderPass);

    virtual ~VulkanImGuiRender();

    void init(RenderState& state,VulkanRenderScene* scene,
             std::shared_ptr<VulkanSwapChain> swapChain);

    void update(RenderState& state,
                VulkanRenderScene* scene);

    void resize(std::shared_ptr<VulkanSwapChain> swapChain);

    void render(RenderState& state,
                           VulkanRenderScene* scene,
                           const VulkanRenderFrame& frame);

    void shutdown();

private:
    VulkanRenderContext context;
    VkRenderPass renderPass;
    VkExtent2D extent;
};


#endif //GAMEENGINE_VULKANIMGUIRENDER_H
