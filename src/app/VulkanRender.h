//
// Created by y123456 on 2021/10/10.
//

#ifndef GAMEENGINE_VULKANRENDER_H
#define GAMEENGINE_VULKANRENDER_H

#include "VulkanCubemapRender.h"
#include "VulkanTexture2DRender.h"
#include <volk.h>
#include <vector>

struct VulkanRenderFrame;
class VulkanTexture;
class VulkanRenderScene;
class VulkanSwapChain;
class VulkanRenderScene;
struct RenderState;
class VulkanContext;

class VulkanRender {
private:
    const VulkanContext* context;
    VkExtent2D extent;

    //TODO swapchain descriptorSetLayout
    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkPipelineLayout  pipelineLayout{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};

    VulkanCubeMapRender hdriToCubeRenderer;
    VulkanCubeMapRender diffuseIrradianceRenderer;
    std::vector<VulkanCubeMapRender*> cubeToPrefilteredRenderers;
    VulkanTexture2DRender brdfRender;


    std::shared_ptr <VulkanTexture> brdfBaked;
    std::shared_ptr <VulkanTexture> environmentCubemap;
    std::shared_ptr <VulkanTexture> diffuseIrradianceCubemap;

    VkPipeline pbrPipeline{VK_NULL_HANDLE};
    VkPipeline skyboxPipeline{VK_NULL_HANDLE};

    VkDescriptorSetLayout sceneDescriptorSetLayout{VK_NULL_HANDLE};
    VkDescriptorSet sceneDescriptorSet{VK_NULL_HANDLE};



public:
    explicit VulkanRender(const VulkanContext* ctx,
                          VkExtent2D extent,
                          VkDescriptorSetLayout layout,
                          VkRenderPass renderPass);
    virtual ~VulkanRender();

    void setextent(int width,int height){extent.width =width;extent.height= height;}

    void init(VulkanRenderScene* scene);

    void update(RenderState& state,VulkanRenderScene *scene);

    void render(VulkanRenderScene *scene, const VulkanRenderFrame& frame);

    void shutdown();

    void resize(const std::shared_ptr<VulkanSwapChain> swapChain);

    void reload(VulkanRenderScene* scene);

    void setEnvironment(std::shared_ptr< VulkanTexture> texture);

    std::shared_ptr<VulkanTexture> getBakedBRDF() const{return brdfBaked;}

private:

};


#endif //GAMEENGINE_VULKANRENDER_H
