//
// Created by y123456 on 2021/10/10.
//

#include "VulkanRender.h"
#include "VulkanSwapChain.h"
#include "VulkanGraphicsPipelineBuilder.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanPipelineLayoutBuilder.h"
#include "VulkanRenderPassBuilder.h"
#include "VulkanRenderScene.h"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include "Macro.h"
#include <imgui_impl_vulkan.h>
#include "config.h"
#include "VulkanUtils.h"
#include "VulkanTexture.h"

VulkanRender::VulkanRender(VulkanRenderContext &ctx)
:context(ctx),
hdriToCubeRenderer(ctx),
diffuseIrradianceRenderer(ctx),
environmentCubemap(new VulkanTexture(ctx)),
diffuseIrradianceCubemap(new VulkanTexture(ctx))
{

}

VulkanRender::~VulkanRender() {
    shutdown();
}

void VulkanRender::init(VulkanRenderScene* scene,VkExtent2D extent,VkDescriptorSetLayout layout,VkRenderPass renderPass) {
    this->extent =extent;
    this->renderPass = renderPass;
    this->descriptorSetLayout = layout;

    std::shared_ptr<VulkanShader> vertShader = scene->getPBRVertexShader();
    std::shared_ptr<VulkanShader> fragShader = scene->getPBRFragmentShader();
    std::shared_ptr<VulkanShader> skyboxVertexShader = scene->getSkyboxVertexShader();
    std::shared_ptr<VulkanShader> skyboxFragmentShader = scene->getSkyboxFragmentShader();

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
    scissor.extent =extent;

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
    pipelineLayoutBuilder.addDescriptorSetLayout(layout);
    pipelineLayoutBuilder.addDescriptorSetLayout(sceneDescriptorSetLayout);
    pipelineLayout = pipelineLayoutBuilder.build();


    VulkanGraphicsPipelineBuilder pbrpipelineBuilder(context,pipelineLayout,renderPass);
    pbrpipelineBuilder.addShaderStage(vertShader->getShaderModule(), VK_SHADER_STAGE_VERTEX_BIT);
    pbrpipelineBuilder.addShaderStage(fragShader->getShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT);
    pbrpipelineBuilder.addVertexInput(VulkanMesh::getVertexInputBindingDescription(),VulkanMesh::getAttributeDescriptions());
    pbrpipelineBuilder.setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pbrpipelineBuilder.addViewport(viewport);
    pbrpipelineBuilder.addScissor(scissor);
    pbrpipelineBuilder.setRasterizerState(false, false, VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    pbrpipelineBuilder.setMultisampleState(context.maxMSAASamples, true);
    pbrpipelineBuilder.setDepthStencilState(true, true, VK_COMPARE_OP_LESS),
    pbrpipelineBuilder.addBlendColorAttachment();
    pbrPipeline =  pbrpipelineBuilder.build();

    VulkanGraphicsPipelineBuilder skyboxPipelineBuilder(context, pipelineLayout, renderPass);
    skyboxPipeline = skyboxPipelineBuilder
            .addShaderStage(skyboxVertexShader->getShaderModule(), VK_SHADER_STAGE_VERTEX_BIT)
            .addShaderStage(skyboxFragmentShader->getShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT)
            .addVertexInput(VulkanMesh::getVertexInputBindingDescription(), VulkanMesh::getAttributeDescriptions())
            .setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .addViewport(viewport)
            .addScissor(scissor)
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

    initEnvironment(scene);

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

    sceneDescriptorSetLayout = VK_NULL_HANDLE;

}

void VulkanRender::update(const VulkanRenderScene *scene) {
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

// ImGui
//    static float f = 0.0f;
//    static int counter = 0;
//    static bool show_demo_window = false;
//    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

//    if (show_demo_window)
//        ImGui::ShowDemoWindow(&show_demo_window);
//
//    ImGui::Begin("Material Parameters");
//
//    ImGui::Checkbox("Demo Window", &show_demo_window);
//
//    ImGui::SliderFloat("Lerp User Material", &state.lerpUserValues, 0.0f, 1.0f);
//    ImGui::SliderFloat("Metalness", &state.userMetalness, 0.0f, 1.0f);
//    ImGui::SliderFloat("Roughness", &state.userRoughness, 0.0f, 1.0f);
//
//    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
//    ImGui::End();

}

VkCommandBuffer VulkanRender::render(VulkanRenderScene *scene, const VulkanRenderFrame& frame ) {
    VkCommandBuffer commandBuffer = frame.commandBuffer;
    VkFramebuffer frameBuffer = frame.frameBuffer;
    VkDeviceMemory uniformBufferMemory = frame.uniformBuffersMemory;
    VkDescriptorSet descriptorSet = frame.swapchainDescriptorSet;
    //VkRenderPass

    //Copy Render State to ubo
    void *ubo = nullptr;
    vkMapMemory(context.device_, uniformBufferMemory, 0, sizeof(RenderState), 0, &ubo);
    memcpy(ubo, &state, sizeof(RenderState));
    vkUnmapMemory(context.device_, uniformBufferMemory);


    //do actual drawing
    VK_CHECK(vkResetCommandBuffer(commandBuffer, 0),"Can't reset command buffer");
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo),"Can't begin recording command buffer");

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

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline);
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

    VK_CHECK(vkEndCommandBuffer(commandBuffer),"Can't record command buffer");

    return commandBuffer;
}

void VulkanRender::initEnvironment(VulkanRenderScene* scene){
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
    setEnvironment(scene,currentEnvironment);
}

void VulkanRender::setEnvironment(VulkanRenderScene *scene, int index) {
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


