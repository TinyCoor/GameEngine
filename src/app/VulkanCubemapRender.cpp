//
// Created by y123456 on 2021/10/10.
//

#include <glm/gtc/matrix_transform.hpp>
#include "VulkanCubemapRender.h"
#include "VulkanMesh.h"
#include "VulkanGraphicsPipelineBuilder.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanPipelineLayoutBuilder.h"
#include "VulkanRenderPassBuilder.h"
#include "VulkanUtils.h"
#include "VulkanShader.h"
#include "Macro.h"
#include "VulkanTexture.h"

struct CubemapFaceOrientationData{
    glm::mat4 faces[6];
};

VulkanCubeMapRender::VulkanCubeMapRender(const VulkanContext* ctx)
:context(ctx),renderQuad(new VulkanMesh(ctx)){}

void VulkanCubeMapRender::init(std::shared_ptr <VulkanShader> vertShader,
                               std::shared_ptr <VulkanShader> fragShader,
                               std::shared_ptr <VulkanTexture> targetTexture) {

    renderQuad->createQuad(2.0f);

    for (int i = 0; i < 6; ++i) {
        faceViews[i]=VulkanUtils::createImageView(
                context->device,
                targetTexture->getImage(),
                targetTexture->getImageFormat(),
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_VIEW_TYPE_2D,
                0,targetTexture->getNumMiplevels(),
                i,1
                );
    }

    targetExtent.width = targetTexture->getWidth();
    targetExtent.height = targetTexture->getHeight();


    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)targetExtent.width;
    viewport.height = (float)targetExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //create scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = targetTexture->getWidth();
    scissor.extent.height= targetTexture->getHeight();

    VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VulkanDescriptorSetLayoutBuilder descriptorSetLayoutBuilder(context);
    descriptorSetLayout = descriptorSetLayoutBuilder
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .build();


    VulkanRenderPassBuilder renderPassBuilder(context);
    renderPass = renderPassBuilder
            .addColorAttachment(targetTexture->getImageFormat(), VK_SAMPLE_COUNT_1_BIT,VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_STORE)
            .addColorAttachment(targetTexture->getImageFormat(), VK_SAMPLE_COUNT_1_BIT,VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_STORE)
            .addColorAttachment(targetTexture->getImageFormat(), VK_SAMPLE_COUNT_1_BIT,VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_STORE)
            .addColorAttachment(targetTexture->getImageFormat(), VK_SAMPLE_COUNT_1_BIT,VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_STORE)
            .addColorAttachment(targetTexture->getImageFormat(), VK_SAMPLE_COUNT_1_BIT,VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_STORE)
            .addColorAttachment(targetTexture->getImageFormat(), VK_SAMPLE_COUNT_1_BIT,VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_STORE)
            .addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
            .addColorAttachmentReference(0, 0)
            .addColorAttachmentReference(0, 1)
            .addColorAttachmentReference(0, 2)
            .addColorAttachmentReference(0, 3)
            .addColorAttachmentReference(0, 4)
            .addColorAttachmentReference(0, 5)
            .build();


    VulkanPipelineLayoutBuilder pipelineLayoutBuilder(context);
    pipelineLayoutBuilder.addDescriptorSetLayout(descriptorSetLayout);
    pipelineLayout = pipelineLayoutBuilder.build();


    VulkanGraphicsPipelineBuilder pipelineBuilder(context,pipelineLayout,renderPass);
    pipeline = pipelineBuilder
            .addShaderStage(vertShader->getShaderModule(), VK_SHADER_STAGE_VERTEX_BIT)
            .addShaderStage(fragShader->getShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT)
            .addVertexInput(VulkanMesh::getVertexInputBindingDescription(), VulkanMesh::getAttributeDescriptions())
            .setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .addViewport(viewport)
            .addScissor(scissor)
            .setRasterizerState(false, false, VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
            .setMultisampleState(VK_SAMPLE_COUNT_1_BIT)
            .setDepthStencilState(false, false, VK_COMPARE_OP_LESS)
            .addBlendColorAttachment()
            .addBlendColorAttachment()
            .addBlendColorAttachment()
            .addBlendColorAttachment()
            .addBlendColorAttachment()
            .addBlendColorAttachment()
            .build();


    VkDeviceSize uboSize =sizeof(CubemapFaceOrientationData);
    VulkanUtils::createBuffer(
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
    descriptorSetAllocInfo.descriptorPool = context->descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
    VK_CHECK(vkAllocateDescriptorSets(context->device, &descriptorSetAllocInfo, &descriptorSet),"Can't allocate descriptor sets");


    //create frameBuffer
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 6;
    framebufferInfo.pAttachments = faceViews;
    framebufferInfo.width = targetExtent.width;
    framebufferInfo.height =targetExtent.height;
    framebufferInfo.layers = 1;
    VK_CHECK(vkCreateFramebuffer(context->device, &framebufferInfo, nullptr, &frameBuffer),"Can't create framebuffer");

    // Create command buffers
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = context->commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;
    VK_CHECK(vkAllocateCommandBuffers(context->device, &allocateInfo, &commandBuffer),"Can't create command buffers");

    //fill uniform buffer
    CubemapFaceOrientationData* ubo{nullptr};
    vkMapMemory(context->device, uniformBuffersMemory, 0, sizeof(CubemapFaceOrientationData), 0,reinterpret_cast<void **>(&ubo));

    const glm::mat4 &translateZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    const glm::vec3 faceDirs[6] = {
            glm::vec3( 1.0f,  0.0f,  0.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3( 0.0f,  1.0f,  0.0f),
            glm::vec3( 0.0f, -1.0f,  0.0f),
            glm::vec3( 0.0f,  0.0f,  1.0f),
            glm::vec3( 0.0f,  0.0f, -1.0f),
    };

    const glm::vec3 faceUps[6] = {
            glm::vec3( 0.0f,  0.0f, -1.0f),
            glm::vec3( 0.0f,  0.0f,  1.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3( 0.0f, -1.0f,  0.0f),
            glm::vec3( 0.0f, -1.0f,  0.0f),
    };

    const glm::mat4 faceRotations[6] = {
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            glm::mat4(1.0f),
            glm::mat4(1.0f),
            glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    };
    for (int i = 0; i < 6; i++)
        ubo->faces[i] = faceRotations[i] * glm::lookAtRH(glm::vec3(0.0f), faceDirs[i], faceUps[i]) * translateZ;


    vkUnmapMemory(context->device, uniformBuffersMemory);

    VulkanUtils::bindUniformBuffer(
            context->device,
            descriptorSet,
            0,
            uniformBuffer,
            0,
            uboSize
    );


    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;
    VK_CHECK(vkCreateFence(context->device, &fenceInfo, nullptr, &fence) ,"Can't create fence");


}



void VulkanCubeMapRender::shutdown() {

    vkDestroyFramebuffer(context->device, frameBuffer, nullptr);
    frameBuffer= VK_NULL_HANDLE;
    vkDestroyBuffer(context->device, uniformBuffer, nullptr);
    uniformBuffer=VK_NULL_HANDLE;
    vkFreeMemory(context->device, uniformBuffersMemory, nullptr);
    uniformBuffersMemory=VK_NULL_HANDLE;

    for (int i = 0; i < 6; i++)
    {
        vkDestroyImageView(context->device, faceViews[i], nullptr);
        faceViews[i] = VK_NULL_HANDLE;
    }

    VK_DESTROY_OBJECT(vkDestroyDescriptorSetLayout(context->device,descriptorSetLayout, nullptr),descriptorSetLayout);

    vkDestroyRenderPass(context->device,renderPass, nullptr);
    renderPass = VK_NULL_HANDLE;

    vkDestroyPipelineLayout(context->device,pipelineLayout, nullptr);
    pipelineLayout = VK_NULL_HANDLE;

    vkDestroyPipeline(context->device,pipeline, nullptr);
    pipeline = VK_NULL_HANDLE;

    vkDestroyFence(context->device, fence, nullptr);
    fence = VK_NULL_HANDLE;
    renderQuad->clearGPUData();
    renderQuad->clearCPUData();
}

void VulkanCubeMapRender::render(std::shared_ptr <VulkanTexture> inputTexture) {

   VulkanUtils::bindCombinedImageSampler(
            context->device,
            descriptorSet,
            1,
            inputTexture->getImageView(),
            inputTexture->getSampler()
    );

    // Record command buffers
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
    renderPassInfo.renderArea.extent.width = targetExtent.width;
    renderPassInfo.renderArea.extent.height = targetExtent.height;

    VkClearValue clearValues[6];
    for (int i = 0; i < 6; i++)
    {
        clearValues[i] = {};
        clearValues[i].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    }
    renderPassInfo.clearValueCount = 6;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

    VkBuffer vertexBuffers[] = {renderQuad->getVertexBuffer()};
    VkBuffer indexBuffer = renderQuad->getIndexBuffer();
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, renderQuad->getNumIndices(), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
    VK_CHECK(vkEndCommandBuffer(commandBuffer),"Can't record command buffer");
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount=1;
    submitInfo.pCommandBuffers =&commandBuffer;

    VK_CHECK( vkResetFences(context->device,1,&fence),"Reset Fence Failed");
    VK_CHECK( vkQueueSubmit(context->graphicsQueue,1,&submitInfo,fence),"Submit Queue Failed");
    VK_CHECK(vkWaitForFences(context->device, 1, &fence, VK_TRUE, 100000000000),"Can't wait for a fence");

}
