//
// Created by 12132 on 2021/11/9.
//

#include "VulkanTexture2DRender.h"
#include "VulkanMesh.h"
#include "VulkanTexture.h"
#include "VulkanPipelineLayoutBuilder.h"
#include "VulkanRenderPassBuilder.h"
#include "VulkanGraphicsPipelineBuilder.h"
#include "VulkanShader.h"
#include "Macro.h"


VulkanTexture2DRender::VulkanTexture2DRender(const VulkanContext* ctx):
context(ctx),
renderQuad(new VulkanMesh(ctx)) {

}

void VulkanTexture2DRender::init(std::shared_ptr<VulkanShader> vertShader, std::shared_ptr<VulkanShader> fragShader,
                                 std::shared_ptr<VulkanTexture> targetTexture) {

    renderQuad->createQuad(2.0f);

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

    VulkanRenderPassBuilder renderPassBuilder(context);
    renderPass = renderPassBuilder
           .addColorAttachment(targetTexture->getImageFormat(), VK_SAMPLE_COUNT_1_BIT,VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_STORE)
            .addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
            .addColorAttachmentReference(0, 0)
            .build();


    VulkanPipelineLayoutBuilder pipelineLayoutBuilder(context);
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
            .build();

    //create frameBuffer
    auto view= targetTexture->getImageView();
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &view;
    framebufferInfo.width = targetExtent.width;
    framebufferInfo.height =targetExtent.height;
    framebufferInfo.layers = 1;
    VK_CHECK(vkCreateFramebuffer(context->Device(), &framebufferInfo, nullptr, &framebuffer),"Can't create framebuffer");

    // Create command buffers
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = context->CommandPool();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;
    VK_CHECK(vkAllocateCommandBuffers(context->Device(), &allocateInfo, &commandBuffer),"Can't create command buffers");


    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;
    VK_CHECK(vkCreateFence(context->Device(), &fenceInfo, nullptr, &fence) ,"Can't create fence");

}

void VulkanTexture2DRender::shutdown() {
    renderQuad->clearGPUData();
    renderQuad->clearCPUData();

    vkDestroyFramebuffer(context->Device(),framebuffer, nullptr);
    framebuffer=VK_NULL_HANDLE;


    vkFreeCommandBuffers(context->Device(),context->CommandPool(),1,&commandBuffer);
    commandBuffer =VK_NULL_HANDLE;

    vkDestroyRenderPass(context->Device(),renderPass, nullptr);
    renderPass = VK_NULL_HANDLE;

    vkDestroyPipelineLayout(context->Device(),pipelineLayout, nullptr);
    pipelineLayout = VK_NULL_HANDLE;

    vkDestroyPipeline(context->Device(),pipeline, nullptr);
    pipeline = VK_NULL_HANDLE;

    vkDestroyFence(context->Device(), fence, nullptr);
    fence = VK_NULL_HANDLE;
}

void VulkanTexture2DRender::render() {

    // Record command buffers
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo),"Can't begin recording command buffer");

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent.width = targetExtent.width;
    renderPassInfo.renderArea.extent.height = targetExtent.height;

    VkClearValue clearValue {};
    clearValue.color= {0.f,0.f,0.f,1.f};
    renderPassInfo.clearValueCount =  1;
    renderPassInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

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

    VK_CHECK( vkResetFences(context->Device(),1,&fence),"Reset Fence Failed");
    VK_CHECK( vkQueueSubmit(context->GraphicsQueue(),1,&submitInfo,fence),"Submit Queue Failed");
    VK_CHECK(vkWaitForFences(context->Device(), 1, &fence, VK_TRUE, 100000000000),"Can't wait for a fence");

}
