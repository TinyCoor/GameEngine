//
// Created by y123456 on 2021/10/10.
//

#ifndef GAMEENGINE_VULKANRENDER_H
#define GAMEENGINE_VULKANRENDER_H

#include "VulkanRenderContext.h"
#include "VulkanCubemapRender.h"
#include <volk.h>
#include <vector>

struct VulkanRenderFrame;
class VulkanTexture;
class VulkanRenderScene;
class VulkanSwapChain;
class VulkanRenderScene;
struct RenderState;

class VulkanRender {
private:
    VulkanRenderContext context;
    VkExtent2D extent;
    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkPipelineLayout  pipelineLayout{VK_NULL_HANDLE};

    VulkanCubeMapRender hdriToCubeRenderer;
    VulkanCubeMapRender diffuseIrradianceRenderer;

    std::shared_ptr<VulkanTexture> environmentCubemap;
    std::shared_ptr <VulkanTexture> diffuseIrradianceCubemap;

    VkPipeline pbrPipeline{VK_NULL_HANDLE};
    VkPipeline skyboxPipeline{VK_NULL_HANDLE};


    VkDescriptorSetLayout sceneDescriptorSetLayout{VK_NULL_HANDLE};

    //TODO swapchain descriptorSetLayout
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    VkDescriptorSet sceneDescriptorSet{VK_NULL_HANDLE};

    uint32_t currentEnvironment =0;

public:
    explicit VulkanRender(VulkanRenderContext& ctx,
                          VkExtent2D extent,
                          VkDescriptorSetLayout layout,
                          VkRenderPass renderPass);
    virtual ~VulkanRender();

    void setextent(int width,int height){extent.width =width;extent.height= height;}

    void init(RenderState& state, VulkanRenderScene* scene);

    void update(RenderState& state,VulkanRenderScene *scene);

    void render(RenderState& state,VulkanRenderScene *scene, const VulkanRenderFrame& frame);

    void shutdown();

    void resize(const std::shared_ptr<VulkanSwapChain> swapChain);

    void reload(RenderState& state,VulkanRenderScene* scene);


private:
    void initEnvironment(RenderState& state,VulkanRenderScene* scene);
    void setEnvironment(RenderState& state,VulkanRenderScene* scene,int index);
};


#endif //GAMEENGINE_VULKANRENDER_H
