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
#include "driver.h"


using namespace render::backend::vulkan;

void VulkanTexture2DRender::init(VulkanShader &vertShader,
                                 VulkanShader &fragShader,
                                 VulkanTexture &target_texture) {

    quad.createQuad(2.0f);

    target_extent.width = target_texture.getWidth();
    target_extent.height = target_texture.getHeight();


    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)target_extent.width;
    viewport.height = (float)target_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //create scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = target_extent.width;
    scissor.extent.height= target_extent.height;

    VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VulkanRenderPassBuilder renderPassBuilder(context);
    render_pass = renderPassBuilder
           .addColorAttachment(target_texture.getImageFormat(), VK_SAMPLE_COUNT_1_BIT,VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_STORE)
            .addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
            .addColorAttachmentReference(0, 0)
            .build();


    VulkanPipelineLayoutBuilder pipelineLayoutBuilder(context);
    pipeline_layout = pipelineLayoutBuilder.build();


    VulkanGraphicsPipelineBuilder pipelineBuilder(context,pipeline_layout,render_pass);
    pipeline = pipelineBuilder
            .addShaderStage(vertShader.getShaderModule(), VK_SHADER_STAGE_VERTEX_BIT)
            .addShaderStage(fragShader.getShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT)
            .addVertexInput(VulkanMesh::getVertexInputBindingDescription(), VulkanMesh::getAttributeDescriptions())
            .setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .addViewport(viewport)
            .addScissor(scissor)
            .setRasterizerState(false, false, VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
            .setMultisampleState(VK_SAMPLE_COUNT_1_BIT)
            .setDepthStencilState(false, false, VK_COMPARE_OP_LESS)
            .addBlendColorAttachment()
            .build();


  // Create framebuffer
       render::backend::FrameBufferColorAttachment attachments[] =
      {
          { target_texture.getBackend(), 0, 1, 0, 1},
      };

    framebuffer = driver->createFrameBuffer(1, attachments);

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
    quad.clearGPUData();
    quad.clearCPUData();

    driver->destroyFrameBuffer(framebuffer);

    vkFreeCommandBuffers(context->Device(),context->CommandPool(),1,&commandBuffer);
    commandBuffer =VK_NULL_HANDLE;

    vkDestroyRenderPass(context->Device(),render_pass, nullptr);
    render_pass = VK_NULL_HANDLE;

    vkDestroyPipelineLayout(context->Device(),pipeline_layout, nullptr);
    pipeline_layout = VK_NULL_HANDLE;

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
    renderPassInfo.renderPass = render_pass;
    renderPassInfo.framebuffer =static_cast<render::backend::vulkan::FrameBuffer *>(framebuffer)->framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent.width = target_extent.width;
    renderPassInfo.renderArea.extent.height = target_extent.height;

    VkClearValue clearValue {};
    clearValue.color= {0.f,0.f,0.f,1.f};
    renderPassInfo.clearValueCount =  1;
    renderPassInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkBuffer vertexBuffers[] = {quad.getVertexBuffer()};
    VkBuffer indexBuffer = quad.getIndexBuffer();
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, quad.getNumIndices(), 1, 0, 0, 0);

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
