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

    void init(VulkanRenderScene* scene,
             std::shared_ptr<VulkanSwapChain> swapChain);

    void shutdown();

    void resize(std::shared_ptr<VulkanSwapChain> swapChain);

    void render(VulkanRenderScene* scene,const VulkanRenderFrame& frame);


private:
    VulkanRenderContext context;
    VkRenderPass renderPass;
    VkExtent2D extent;
};


#endif //GAMEENGINE_VULKANIMGUIRENDER_H
