//
// Created by y123456 on 2021/10/10.
//

#ifndef GAMEENGINE_VULKANRENDER_H
#define GAMEENGINE_VULKANRENDER_H

#include "VulkanRenderScene.h"
#include "VulkanRenderContext.h"
#include "VulkanCubemapRender.h"
#include <volk.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <glm/glm.hpp>

struct VulkanRenderFrame;
class VulkanTexture;
class VulkanRenderScene;
class VulkanSwapChain;

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

    VulkanCubeMapRender hdriToCubeRenderer;
    VulkanCubeMapRender diffuseIrradianceRenderer;

    std::shared_ptr<VulkanTexture> environmentCubemap;
    std::shared_ptr <VulkanTexture> diffuseIrradianceCubemap;

    VkPipelineLayout  pipelineLayout{VK_NULL_HANDLE};


    VkPipeline pbrPipeline{VK_NULL_HANDLE};
    VkPipeline skyboxPipeline{VK_NULL_HANDLE};


    VkDescriptorSetLayout sceneDescriptorSetLayout{VK_NULL_HANDLE};
    VkDescriptorSet sceneDescriptorSet{VK_NULL_HANDLE};
    //TODO
    VkExtent2D extent;
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;

    int currentEnvironment{0};
    RenderState state;


public:
    explicit VulkanRender(VulkanRenderContext& ctx);
    virtual ~VulkanRender();

    void init(VulkanRenderScene* scene,VkExtent2D extent,VkDescriptorSetLayout layout,VkRenderPass renderPass);

    void initEnvironment(VulkanRenderScene* scene);

    void setEnvironment(VulkanRenderScene* scene,int index);

    void update(const VulkanRenderScene *scene);
    VkCommandBuffer render(VulkanRenderScene *scene, const VulkanRenderFrame& frame);

    void shutdown();
};


#endif //GAMEENGINE_VULKANRENDER_H
