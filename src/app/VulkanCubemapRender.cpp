//
// Created by y123456 on 2021/10/10.
//

#include "VulkanCubemapRender.h"
#include "VulkanGraphicsPipelineBuilder.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanPipelineLayoutBuilder.h"
#include "VulkanRenderPassBuilder.h"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

#include "Macro.h"


void VulkanCubeMapRender::init(VkExtent2D extent) {
    VulkanTexture* inputHDR= nullptr;
    VulkanShader* cubemapFragmentShader= nullptr;
    VulkanShader* cubemapVertexShader = nullptr;

    VkFormat inputImageFormat = inputHDR->getImageFormat();
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //create scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;


    VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VulkanDescriptorSetLayoutBuilder descriptorSetLayoutBuilder(context);
    descriptorSetLayout = descriptorSetLayoutBuilder
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .build();


    VulkanRenderPassBuilder renderPassBuilder(context);
    renderPassBuilder.addColorAttachment(inputImageFormat,VK_SAMPLE_COUNT_1_BIT);
    renderPassBuilder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS);
    renderPassBuilder.addColorAttachmentReference(0,0);
    renderPass = renderPassBuilder.build();

    VulkanPipelineLayoutBuilder pipelineLayoutBuilder(context);
    pipelineLayoutBuilder.addDescriptorSetLayout(descriptorSetLayout);
    pbrPipelineLayout = pipelineLayoutBuilder.build();


    VulkanGraphicsPipelineBuilder pipelineBuilder(context,pbrPipelineLayout,renderPass);
    //VulkanGraphicsPipelineBuilder pipelineBuilder(context,descriptorSetLayout,renderPass);
    pipelineBuilder.addShaderStage(cubemapVertexShader->getShaderModule(), VK_SHADER_STAGE_VERTEX_BIT);
    pipelineBuilder.addShaderStage(cubemapFragmentShader->getShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT);
    pipelineBuilder.addVertexInput(VulkanMesh::getVertexInputBindingDescription(),VulkanMesh::getAttributeDescriptions());
    pipelineBuilder.setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineBuilder.addViewport(viewport);
    pipelineBuilder.addScissor(scissor);
    pipelineBuilder.setRasterizerState(false, false, VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    pipelineBuilder.setMultisampleState(VK_SAMPLE_COUNT_1_BIT);
    pipelineBuilder.setDepthStencilState(true, true, VK_COMPARE_OP_LESS),
    pipelineBuilder.addBlendColorAttachment();
    pbrPipeline =  pipelineBuilder.build();


    //TODO
    VkDeviceSize uboSize =0; ///

    vulkanUtils::createBuffer(
            context,
            uboSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            uniformBuffer,
            uniformBuffersMemory
    );
    // Create descriptor sets
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = context.descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(context.device_, &descriptorSetAllocInfo, &descriptorSet) != VK_SUCCESS)
        throw std::runtime_error("Can't allocate descriptor sets");


    vulkanUtils::bindUniformBuffer(
            context,
            descriptorSet,
            0,
            uniformBuffer,
            0,
            sizeof(SharedRenderState)
    );

    vulkanUtils::bindCombinedImageSampler(
            context,
            descriptorSet,
            1,
            inputHDR->getImageView(),
            inputHDR->getSampler()
    );

    VkImageView cubemapFaceView =VK_NULL_HANDLE;//TODO
    std::array<VkImageView, 1> attachments = {
            cubemapFaceView,
    };

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = extent.width;
    framebufferInfo.height =extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(context.device_, &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS)
        throw std::runtime_error("Can't create framebuffer");

    // Create command buffers
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = context.commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(context.device_, &allocateInfo, &commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Can't create command buffers");

    // Record command buffers
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Can't begin recording command buffer");

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = frameBuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;

    VkClearValue clearValue = {};
    clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues =&clearValue;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
//TODO
//    const VulkanMesh &skybox = scene->getSkyboxMesh();
//    VkBuffer vertexBuffers[] = {skybox.getVertexBuffer()};
//    VkBuffer indexBuffer = skybox.getIndexBuffer();
//    VkDeviceSize offsets[] = {0};
//
//    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
//    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
//
//    vkCmdDrawIndexed(commandBuffer, skybox.getNumIndices(), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
    VK_CHECK(vkEndCommandBuffer(commandBuffer),"Can't record command buffer");
}


void VulkanCubeMapRender::shutdown() {
    vkDestroyFramebuffer(context.device_, frameBuffer, nullptr);
    frameBuffer= VK_NULL_HANDLE;
    vkDestroyBuffer(context.device_, uniformBuffer, nullptr);
    uniformBuffer=VK_NULL_HANDLE;
    vkFreeMemory(context.device_, uniformBuffersMemory, nullptr);
    uniformBuffersMemory=VK_NULL_HANDLE;

    VK_DESTROY_OBJECT(vkDestroyDescriptorSetLayout(context.device_,descriptorSetLayout, nullptr),descriptorSetLayout);

    vkDestroyRenderPass(context.device_,renderPass, nullptr);
    renderPass = VK_NULL_HANDLE;

    vkDestroyPipelineLayout(context.device_,pbrPipelineLayout, nullptr);
    pbrPipelineLayout = VK_NULL_HANDLE;

    vkDestroyPipeline(context.device_,pbrPipeline, nullptr);
    pbrPipeline = VK_NULL_HANDLE;
}

VkCommandBuffer VulkanCubeMapRender::render() {
    return commandBuffer;
}
