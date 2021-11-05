//
// Created by y123456 on 2021/10/10.
//

#include "VulkanRender.h"
#include "VulkanGraphicsPipelineBuilder.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanPipelineLayoutBuilder.h"
#include "VulkanRenderPassBuilder.h"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include "Macro.h"
#include <imgui_impl_vulkan.h>
#include "config.h"
#include "VulkanUtils.h"



void VulkanRender::init(VulkanRenderScene* scene) {

    std::shared_ptr<VulkanShader> vertShader = scene->getPBRVertexShader();
    std::shared_ptr<VulkanShader> fragShader = scene->getPBRFragmentShader();
    std::shared_ptr<VulkanShader> skyboxVertexShader = scene->getSkyboxVertexShader();
    std::shared_ptr<VulkanShader> skyboxFragmentShader = scene->getSkyboxFragmentShader();

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainContext.extend.width;
    viewport.height = (float)swapChainContext.extend.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //create scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainContext.extend;


    VulkanDescriptorSetLayoutBuilder sceneDescriptorSetLayoutBuilder(context);
    VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    sceneDescriptorSetLayout = sceneDescriptorSetLayoutBuilder
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .build();

    VulkanDescriptorSetLayoutBuilder swapchainDescriptorSetLayoutBuilder(context);
    swapchainDescriptorSetLayout = swapchainDescriptorSetLayoutBuilder
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage)
            .build();


    VulkanRenderPassBuilder renderPassBuilder(context);
    renderPassBuilder.addColorAttachment(swapChainContext.colorFormat, context.maxMSAASamples);
    renderPassBuilder.addColorResolveAttachment(swapChainContext.colorFormat);
    renderPassBuilder.addDepthStencilAttachment(swapChainContext.depthFormat, context.maxMSAASamples);
    renderPassBuilder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS);
    renderPassBuilder.addColorAttachmentReference(0,0);
    renderPassBuilder.addColorResolveAttachmentReference(0,1);
    renderPassBuilder.setDepthStencilAttachment(0,2);
    renderPass = renderPassBuilder.build();

    VulkanPipelineLayoutBuilder pipelineLayoutBuilder(context);
    pipelineLayoutBuilder.addDescriptorSetLayout(swapchainDescriptorSetLayout);
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

    // Create uniform buffers
    VkDeviceSize uboSize = sizeof(RenderState);
    uint32_t imageCount = static_cast<uint32_t>(swapChainContext.imageViews.size());
    uniformBuffers.resize(imageCount);
    uniformBuffersMemory.resize(imageCount);

    for (size_t i = 0; i < imageCount; i++) {
        VulkanUtils::createBuffer(
                context,
                uboSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                uniformBuffers[i],
                uniformBuffersMemory[i]
        );
    }

    //Create SwapChain descriptor sets
    std::vector<VkDescriptorSetLayout> layouts(imageCount,swapchainDescriptorSetLayout);
    swapchainDescriptorSets.resize(imageCount);
    VkDescriptorSetAllocateInfo swapchainDescriptorSetAllocInfo = {};
    swapchainDescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    swapchainDescriptorSetAllocInfo.descriptorPool = context.descriptorPool;
    swapchainDescriptorSetAllocInfo.descriptorSetCount = layouts.size();
    swapchainDescriptorSetAllocInfo.pSetLayouts = layouts.data();

    VK_CHECK(vkAllocateDescriptorSets(context.device_, &swapchainDescriptorSetAllocInfo, swapchainDescriptorSets.data()),
             "Can't allocate swap chain descriptor sets");

    for (int i = 0; i < imageCount; ++i) {
        //Bind UBO from swapchain
        VulkanUtils::bindUniformBuffer(
                context,
                swapchainDescriptorSets[i],
                0,
                uniformBuffers[i],
                0,
                sizeof(RenderState)
        );
    }

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


    // Create framebuffers
    frameBuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++) {
        std::array<VkImageView, 3> attachments = {
                swapChainContext.colorImageView,
                swapChainContext.imageViews[i],
                swapChainContext.depthImageView,
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainContext.extend.width;
        framebufferInfo.height = swapChainContext.extend.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(context.device_, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Can't create framebuffer");
    }

    // Create command buffers
    commandBuffers.resize(imageCount);
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = context.commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    VK_CHECK(vkAllocateCommandBuffers(context.device_, &allocateInfo, commandBuffers.data()),"Can't create command buffers");

//    // Init ImGui bindings for Vulkan
//    {
//        ImGui_ImplVulkan_InitInfo init_info = {};
//        init_info.Instance = context.instance;
//        init_info.PhysicalDevice = context.physicalDevice;
//        init_info.Device = context.device_;
//        init_info.QueueFamily = context.graphicsQueueFamily;
//        init_info.Queue = context.graphicsQueue;
//        init_info.DescriptorPool = context.descriptorPool;
//        init_info.MSAASamples = context.maxMSAASamples;
//        init_info.MinImageCount = static_cast<uint32_t>(swapChainContext.imageViews.size());
//        init_info.ImageCount = static_cast<uint32_t>(swapChainContext.imageViews.size());
//        init_info.Allocator = nullptr;
//
//        //TODO Fix Bug In this Function VkCreateSampler Cause Segmentation
//        ImGui_ImplVulkan_Init(&init_info, renderPass);
//
//        VulkanRenderContext imGuiContext = {};
//        imGuiContext.commandPool = context.commandPool;
//        imGuiContext.descriptorPool = context.descriptorPool;
//        imGuiContext.device_ = context.device_;
//        imGuiContext.graphicsQueue = context.graphicsQueue;
//        imGuiContext.maxMSAASamples = context.maxMSAASamples;
//        imGuiContext.physicalDevice = context.physicalDevice;
//        imGuiContext.presentQueue = context.presentQueue;
//
//        VkCommandBuffer imGuiCommandBuffer = VulkanUtils::beginSingleTimeCommands(imGuiContext);
//        ImGui_ImplVulkan_CreateFontsTexture(imGuiCommandBuffer);
//        VulkanUtils::endSingleTimeCommands(imGuiContext, imGuiCommandBuffer);
//    }
}


void VulkanRender::shutdown() {
    //Shut down imGui
//    ImGui_ImplVulkan_Shutdown();


    hdriToCubeRenderer.shutdown();
    diffuseIrradianceRenderer.shutdown();

    environmentCubemap->clearGPUData();
    diffuseIrradianceCubemap->clearGPUData();

    for (auto framebuffer : frameBuffers) {
        vkDestroyFramebuffer(context.device_, framebuffer, nullptr);
    }
    for (auto uniformBuffer : uniformBuffers) {
        vkDestroyBuffer(context.device_, uniformBuffer, nullptr);
    }
    for (auto memory : uniformBuffersMemory) {
        vkFreeMemory(context.device_, memory, nullptr);
    }

    frameBuffers.clear();
    uniformBuffers.clear();
    uniformBuffersMemory.clear();

    vkDestroyDescriptorSetLayout(context.device_,sceneDescriptorSetLayout, nullptr);
    sceneDescriptorSetLayout= VK_NULL_HANDLE;

    vkDestroyDescriptorSetLayout(context.device_,swapchainDescriptorSetLayout, nullptr);
    sceneDescriptorSetLayout= VK_NULL_HANDLE;

    vkFreeDescriptorSets(context.device_,context.descriptorPool,1,&sceneDescriptorSet);
    vkFreeDescriptorSets(context.device_,context.descriptorPool,swapchainDescriptorSets.size(),swapchainDescriptorSets.data());
    sceneDescriptorSet = VK_NULL_HANDLE;
    swapchainDescriptorSets.clear();

    vkDestroyRenderPass(context.device_,renderPass, nullptr);
    renderPass = VK_NULL_HANDLE;

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

    const float aspect = swapChainContext.extend.width / (float) swapChainContext.extend.height;
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

VkCommandBuffer VulkanRender::render(VulkanRenderScene *scene, uint32_t imageIndex) {
    VkCommandBuffer commandBuffer = commandBuffers[imageIndex];
    VkFramebuffer frameBuffer = frameBuffers[imageIndex];
    VkBuffer uniformBuffer = uniformBuffers[imageIndex];
    VkDeviceMemory uniformBufferMemory = uniformBuffersMemory[imageIndex];
    VkDescriptorSet descriptorSet = swapchainDescriptorSets[imageIndex];

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
    renderPassInfo.renderArea.extent = swapChainContext.extend;

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

//    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

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

    int imageCount = swapChainContext.imageViews.size();
    for (int i = 0; i <imageCount ; ++i) {
        for (int k = 0; k < textures.size(); k++)
            VulkanUtils::bindCombinedImageSampler(
                    context,
                    sceneDescriptorSet,
                    k + 5,
                    textures[k]->getImageView(),
                    textures[k]->getSampler()
            );
    }


}
