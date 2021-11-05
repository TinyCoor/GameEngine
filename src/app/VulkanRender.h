//
// Created by y123456 on 2021/10/10.
//

#ifndef GAMEENGINE_VULKANRENDER_H
#define GAMEENGINE_VULKANRENDER_H

#include "VulkanRenderScene.h"
#include "VulkanRenderContext.h"
#include "VulkanCubemapRender.h"
#include "VulkanTexture.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <glm/glm.hpp>

struct RenderState{
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 cameraPosWS;
    float lerpUserValues {0.0f};
    float userMetalness {0.0f};
    float userRoughness {0.0f};
};

class VulkanRender {
private:
    VulkanRenderContext context;
    VulkanSwapChainContext swapChainContext;

    VulkanCubeMapRender hdriToCubeRenderer;


    VulkanCubeMapRender diffuseIrradianceRenderer;

    std::shared_ptr< VulkanTexture> environmentCubemap;
    std::shared_ptr <VulkanTexture> diffuseIrradianceCubemap;

    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};


    VkPipelineLayout  pipelineLayout{VK_NULL_HANDLE};

    VkPipeline pbrPipeline{VK_NULL_HANDLE};
    VkPipeline skyboxPipeline{VK_NULL_HANDLE};


    std::vector<VkCommandBuffer> commandBuffers{};
    std::vector<VkFramebuffer> frameBuffers{};

    std::vector<VkBuffer> uniformBuffers{};
    std::vector<VkDeviceMemory> uniformBuffersMemory{};


    VkDescriptorSet descriptorSet{};

    int currentEnvironment{0};
    RenderState state;

private:


public:
    explicit VulkanRender(VulkanRenderContext& ctx, VulkanSwapChainContext& swapChainCtx)
                    :context(ctx),swapChainContext(swapChainCtx)
                    , hdriToCubeRenderer(context)
                    , diffuseIrradianceRenderer(context)
                    , environmentCubemap(new VulkanTexture(ctx))
                    , diffuseIrradianceCubemap(new VulkanTexture(ctx)){
    }

    void init(VulkanRenderScene* scene);

    void initEnvironment(VulkanRenderScene* scene);

    void setEnvironment(VulkanRenderScene* scene,int index);

    void update(const VulkanRenderScene *scene);
    VkCommandBuffer render(VulkanRenderScene *scene, uint32_t imageIndex);

    void shutdown();
};


#endif //GAMEENGINE_VULKANRENDER_H
