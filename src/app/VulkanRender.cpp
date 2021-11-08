//
// Created by y123456 on 2021/10/10.
//

#include "VulkanRender.h"
#include "RenderState.h"
#include "VulkanSwapChain.h"
#include "VulkanGraphicsPipelineBuilder.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanPipelineLayoutBuilder.h"
#include "VulkanMesh.h"
#include "VulkanRenderScene.h"
#include "Macro.h"
#include "VulkanUtils.h"
#include "VulkanTexture.h"

#include <glm/gtc/matrix_transform.hpp>

#include <chrono>



VulkanRender::VulkanRender(VulkanRenderContext &ctx,VkExtent2D size,VkDescriptorSetLayout layout,VkRenderPass pass)
:context(ctx),
renderPass(pass),
extent(size),
descriptorSetLayout(layout),
hdriToCubeRenderer(ctx),
diffuseIrradianceRenderer(ctx),
environmentCubemap(new VulkanTexture(ctx)),
diffuseIrradianceCubemap(new VulkanTexture(ctx))
{

}

VulkanRender::~VulkanRender() {
    shutdown();
}

void VulkanRender::init(RenderState& state,VulkanRenderScene* scene) {

    std::shared_ptr<VulkanShader> vertShader = scene->getPBRVertexShader();
    std::shared_ptr<VulkanShader> fragShader = scene->getPBRFragmentShader();
    std::shared_ptr<VulkanShader> skyboxVertexShader = scene->getSkyboxVertexShader();
    std::shared_ptr<VulkanShader> skyboxFragmentShader = scene->getSkyboxFragmentShader();

    VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VulkanDescriptorSetLayoutBuilder sceneDescriptorSetLayoutBuilder(context);
    sceneDescriptorSetLayout = sceneDescriptorSetLayoutBuilder
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .build();

    VulkanPipelineLayoutBuilder pipelineLayoutBuilder(context);
    pipelineLayoutBuilder.addDescriptorSetLayout(descriptorSetLayout);
    pipelineLayoutBuilder.addDescriptorSetLayout(sceneDescriptorSetLayout);
    pipelineLayout = pipelineLayoutBuilder.build();


    VulkanGraphicsPipelineBuilder pbrpipelineBuilder(context,pipelineLayout,renderPass);
    pbrPipeline = pbrpipelineBuilder .addShaderStage(vertShader->getShaderModule(), VK_SHADER_STAGE_VERTEX_BIT)
        .addShaderStage(fragShader->getShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT)
        .addVertexInput(VulkanMesh::getVertexInputBindingDescription(),VulkanMesh::getAttributeDescriptions())
        .setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .addViewport(VkViewport())
        .addScissor(VkRect2D())
        .addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
        .addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
        .setRasterizerState(false, false, VK_POLYGON_MODE_FILL,
                            1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
        .setMultisampleState(context.maxMSAASamples, true)
        .setDepthStencilState(true, true, VK_COMPARE_OP_LESS)
        .addBlendColorAttachment()
        .build();

    VulkanGraphicsPipelineBuilder skyboxPipelineBuilder(context, pipelineLayout, renderPass);
    skyboxPipeline = skyboxPipelineBuilder
            .addShaderStage(skyboxVertexShader->getShaderModule(), VK_SHADER_STAGE_VERTEX_BIT)
            .addShaderStage(skyboxFragmentShader->getShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT)
            .addVertexInput(VulkanMesh::getVertexInputBindingDescription(), VulkanMesh::getAttributeDescriptions())
            .setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .addViewport(VkViewport())
            .addScissor(VkRect2D())
            .addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            .addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            .setRasterizerState(false, false, VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
            .setMultisampleState(context.maxMSAASamples,true)
            .setDepthStencilState(true, true, VK_COMPARE_OP_LESS)
            .addBlendColorAttachment()
            .build();


    // Create scene descriptor sets
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = context.descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &sceneDescriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(context.device_, &descriptorSetAllocInfo, &sceneDescriptorSet),
             "Can't allocate descriptor sets");

    initEnvironment(state,scene);

    std::array<std::shared_ptr<VulkanTexture>, 7> textures =
    {
            scene->getAlbedoTexture(),
            scene->getNormalTexture(),
            scene->getAOTexture(),
            scene->getShadingTexture(),
            scene->getEmissionTexture(),
            environmentCubemap,
            diffuseIrradianceCubemap,
    };

    for (int k = 0; k < textures.size(); k++)
        VulkanUtils::bindCombinedImageSampler(
                context,
                sceneDescriptorSet,
                k,
                textures[k]->getImageView(),
                textures[k]->getSampler()
        );
}


void VulkanRender::shutdown() {

    hdriToCubeRenderer.shutdown();
    diffuseIrradianceRenderer.shutdown();

    environmentCubemap->clearGPUData();
    diffuseIrradianceCubemap->clearGPUData();


    vkDestroyDescriptorSetLayout(context.device_,sceneDescriptorSetLayout, nullptr);
    sceneDescriptorSetLayout= VK_NULL_HANDLE;


    vkDestroyPipelineLayout(context.device_,pipelineLayout, nullptr);
    pipelineLayout = VK_NULL_HANDLE;

    vkDestroyPipeline(context.device_,pbrPipeline, nullptr);
    pbrPipeline = VK_NULL_HANDLE;


    vkDestroyPipeline(context.device_,skyboxPipeline, nullptr);
    skyboxPipeline = VK_NULL_HANDLE;


}

void VulkanRender::update(RenderState& state,VulkanRenderScene *scene) {
    // Render state
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();

    const float rotationSpeed = 0.1f;
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    const glm::vec3 &up = {0.0f, 0.0f, 1.0f};
    const glm::vec3 &zero = {0.0f, 0.0f, 0.0f};

    const float aspect =(float )extent.width /(float ) extent.height;
    const float zNear = 0.1f;
    const float zFar = 1000.0f;

    const glm::vec3 &cameraPos = glm::vec3(2.0f, 2.0f, 2.0f);
    const glm::mat4 &rotation = glm::rotate(glm::mat4(1.0f), time * rotationSpeed * glm::radians(90.0f), up);

    state.world = glm::mat4(1.0f);
    state.view = glm::lookAt(cameraPos, zero, up) * rotation;
    state.proj = glm::perspective(glm::radians(60.0f), aspect, zNear, zFar);
    state.proj[1][1] *= -1;
    state.cameraPosWS = glm::vec3(glm::vec4(cameraPos, 1.0f) * rotation);
    if (currentEnvironment !=state.currentEnvironment){
        currentEnvironment= state.currentEnvironment;
        setEnvironment(state,scene,currentEnvironment);
    }

}

void VulkanRender::render(RenderState& state,VulkanRenderScene *scene, const VulkanRenderFrame& frame ) {
    VkCommandBuffer commandBuffer = frame.commandBuffer;
    VkFramebuffer frameBuffer = frame.frameBuffer;
    VkDeviceMemory uniformBufferMemory = frame.uniformBuffersMemory;
    VkDescriptorSet descriptorSet = frame.swapchainDescriptorSet;


    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = frameBuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;

    std::array<VkClearValue, 3> clearValues = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[2].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();


    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = extent.width;
    viewport.height = extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //create scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent=extent;


    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline);

    vkCmdSetScissor(commandBuffer,0,1,&scissor);
    vkCmdSetViewport(commandBuffer,0,1,&viewport);


    std::array<VkDescriptorSet,2> bindDescriptors={descriptorSet,sceneDescriptorSet };
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, bindDescriptors.size(), bindDescriptors.data(), 0, nullptr);


    {
       auto skybox = scene->getSkyboxMesh();

        VkBuffer vertexBuffers[] = { skybox->getVertexBuffer() };
        VkBuffer indexBuffer = skybox->getIndexBuffer();
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(commandBuffer, skybox->getNumIndices(), 1, 0, 0, 0);
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, bindDescriptors.size(), bindDescriptors.data(), 0, nullptr);
    {
        auto mesh = scene->getMesh();

        VkBuffer vertexBuffers[] = { mesh->getVertexBuffer() };
        VkBuffer indexBuffer = mesh->getIndexBuffer();
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(commandBuffer, mesh->getNumIndices(), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);


}

void VulkanRender::initEnvironment(RenderState& state,VulkanRenderScene* scene){
    environmentCubemap->createCube(VK_FORMAT_R32G32B32A32_SFLOAT, 256, 256, 1);
    diffuseIrradianceCubemap->createCube(VK_FORMAT_R32G32B32A32_SFLOAT, 256, 256, 1);
    hdriToCubeRenderer.init(
                scene->getCubeVertexShader(),
                scene->getHDRToCubeFragmentShader(),
                environmentCubemap
        );

    diffuseIrradianceRenderer.init(
            scene->getCubeVertexShader(),
            scene->getDiffuseToIrridanceShader(),
            diffuseIrradianceCubemap
    );
    setEnvironment(state,scene,state.currentEnvironment);
}

void VulkanRender::setEnvironment(RenderState& state,VulkanRenderScene* scene, int index) {
    {
        VulkanUtils::transitionImageLayout(
                context,
                environmentCubemap->getImage(),
                environmentCubemap->getImageFormat(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                0, environmentCubemap->getNumMiplevels(),
                0, environmentCubemap->getNumLayers()
        );

        hdriToCubeRenderer.render(scene->getHDRTexture(index));

        VulkanUtils::transitionImageLayout(
                context,
                environmentCubemap->getImage(),
                environmentCubemap->getImageFormat(),
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                0, environmentCubemap->getNumMiplevels(),
                0, environmentCubemap->getNumLayers()
        );
    }


    {
        VulkanUtils::transitionImageLayout(
                context,
                diffuseIrradianceCubemap->getImage(),
                diffuseIrradianceCubemap->getImageFormat(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                0, diffuseIrradianceCubemap->getNumMiplevels(),
                0, diffuseIrradianceCubemap->getNumLayers()
        );
   
        diffuseIrradianceRenderer.render(environmentCubemap);

        VulkanUtils::transitionImageLayout(
                context,
                diffuseIrradianceCubemap->getImage(),
                diffuseIrradianceCubemap->getImageFormat(),
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                0, diffuseIrradianceCubemap->getNumMiplevels(),
                0, diffuseIrradianceCubemap->getNumLayers()
        );
    }

    std::array<std::shared_ptr<VulkanTexture>, 2> textures ={
                    environmentCubemap,
                    diffuseIrradianceCubemap,
    };


    for (int k = 0; k < textures.size(); k++)
        VulkanUtils::bindCombinedImageSampler(
                context,
                sceneDescriptorSet,
                k + 5,
                textures[k]->getImageView(),
                textures[k]->getSampler()
        );


}

void VulkanRender::resize(const std::shared_ptr<VulkanSwapChain> swapChain) {
    extent = swapChain->getExtent();

}

void VulkanRender::reload(RenderState &state, VulkanRenderScene *scene) {
    shutdown();
    init(state,scene);
}


