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
namespace render::backend::vulkan {

struct CubemapFaceOrientationData {
    glm::mat4 faces[6];
};

VulkanCubeMapRender::VulkanCubeMapRender(render::backend::Driver *driver)
: driver(driver),quad(driver)
{
    context = static_cast<vulkan::VulkanDriver*>(driver)->GetDevice();
}

void VulkanCubeMapRender::init(VulkanShader &vertShader,
                               VulkanShader &fragShader,
                               VulkanTexture &target_texture,
                               int mip,
                               uint32_t userDataSize)
{
    push_constants_size = userDataSize;
    quad.createQuad(2.0f);

    target_extent.width = target_texture.getWidth(mip);
    target_extent.height = target_texture.getHeight(mip);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) target_extent.width;
    viewport.height = (float) target_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //create scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = target_texture.getWidth();
    scissor.extent.height = target_texture.getHeight();

    VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VulkanDescriptorSetLayoutBuilder descriptorSetLayoutBuilder;
    descriptor_set_layout = descriptorSetLayoutBuilder
        .addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage, 0)
        .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage, 1)
        .build(context->LogicDevice());

    VulkanRenderPassBuilder renderPassBuilder;
    render_pass = renderPassBuilder
        .addColorAttachment(target_texture.getImageFormat(),
                            VK_SAMPLE_COUNT_1_BIT,
                            VK_ATTACHMENT_LOAD_OP_CLEAR,
                            VK_ATTACHMENT_STORE_OP_STORE)
        .addColorAttachment(target_texture.getImageFormat(),
                            VK_SAMPLE_COUNT_1_BIT,
                            VK_ATTACHMENT_LOAD_OP_CLEAR,
                            VK_ATTACHMENT_STORE_OP_STORE)
        .addColorAttachment(target_texture.getImageFormat(),
                            VK_SAMPLE_COUNT_1_BIT,
                            VK_ATTACHMENT_LOAD_OP_CLEAR,
                            VK_ATTACHMENT_STORE_OP_STORE)
        .addColorAttachment(target_texture.getImageFormat(),
                            VK_SAMPLE_COUNT_1_BIT,
                            VK_ATTACHMENT_LOAD_OP_CLEAR,
                            VK_ATTACHMENT_STORE_OP_STORE)
        .addColorAttachment(target_texture.getImageFormat(),
                            VK_SAMPLE_COUNT_1_BIT,
                            VK_ATTACHMENT_LOAD_OP_CLEAR,
                            VK_ATTACHMENT_STORE_OP_STORE)
        .addColorAttachment(target_texture.getImageFormat(),
                            VK_SAMPLE_COUNT_1_BIT,
                            VK_ATTACHMENT_LOAD_OP_CLEAR,
                            VK_ATTACHMENT_STORE_OP_STORE)
        .addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
        .addColorAttachmentReference(0, 0)
        .addColorAttachmentReference(0, 1)
        .addColorAttachmentReference(0, 2)
        .addColorAttachmentReference(0, 3)
        .addColorAttachmentReference(0, 4)
        .addColorAttachmentReference(0, 5)
        .build(context->LogicDevice());

    VulkanPipelineLayoutBuilder pipelineLayoutBuilder;
    pipelineLayoutBuilder.addDescriptorSetLayout(descriptor_set_layout);

    if (push_constants_size > 0) {
        pipelineLayoutBuilder.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 0, push_constants_size);
    }
    pipeline_layout = pipelineLayoutBuilder.build(context->LogicDevice());

    VulkanGraphicsPipelineBuilder pipelineBuilder(pipeline_layout, render_pass);
    pipeline = pipelineBuilder
        .addShaderStage(vertShader.getShaderModule(), VK_SHADER_STAGE_VERTEX_BIT)
        .addShaderStage(fragShader.getShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT)
        .addVertexInput(VulkanMesh::getVertexInputBindingDescription(), VulkanMesh::getAttributeDescriptions())
        .setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .addViewport(viewport)
        .addScissor(scissor)
        .setRasterizerState(false,
                            false,
                            VK_POLYGON_MODE_FILL,
                            1.0f,
                            VK_CULL_MODE_BACK_BIT,
                            VK_FRONT_FACE_COUNTER_CLOCKWISE)
        .setMultisampleState(VK_SAMPLE_COUNT_1_BIT)
        .setDepthStencilState(false, false, VK_COMPARE_OP_LESS)
        .addBlendColorAttachment()
        .addBlendColorAttachment()
        .addBlendColorAttachment()
        .addBlendColorAttachment()
        .addBlendColorAttachment()
        .addBlendColorAttachment()
        .build(context->LogicDevice());

    uint32_t ubo_size = sizeof(CubemapFaceOrientationData);
    uniform_buffer = driver->createUniformBuffer(render::backend::BufferType::DYNAMIC, ubo_size);

    // Create descriptor sets
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = context->DescriptorPool();
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &descriptor_set_layout;
    VK_CHECK(vkAllocateDescriptorSets(context->LogicDevice(), &descriptorSetAllocInfo, &descriptorSet),
             "Can't allocate descriptor sets");


    //create frameBuffer
    // Create framebuffer
    render::backend::FrameBufferAttachmentType type = render::backend::FrameBufferAttachmentType::COLOR;
    render::backend::FrameBufferAttachment attachments[6] =
        {
            {type, target_texture.getTexture(), mip, 1, 0, 1},
            {type, target_texture.getTexture(), mip, 1, 1, 1},
            {type, target_texture.getTexture(), mip, 1, 2, 1},
            {type, target_texture.getTexture(), mip, 1, 3, 1},
            {type, target_texture.getTexture(), mip, 1, 4, 1},
            {type, target_texture.getTexture(), mip, 1, 5, 1},
        };
    framebuffer = driver->createFrameBuffer(6, attachments);

    // Create command buffers
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = context->CommandPool();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;
    VK_CHECK(vkAllocateCommandBuffers(context->LogicDevice(), &allocateInfo, &command_buffer),
             "Can't create command buffers");


    //fill uniform buffer
    CubemapFaceOrientationData *ubo = reinterpret_cast<CubemapFaceOrientationData *>(driver->map(uniform_buffer));

    const glm::mat4 &translateZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    const glm::vec3 faceDirs[6] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
    };

    const glm::vec3 faceUps[6] = {
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
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

    driver->unmap(uniform_buffer);

    VulkanUtils::bindUniformBuffer(
        context->LogicDevice(),
        descriptorSet,
        0,
        static_cast<render::backend::vulkan::UniformBuffer *>(uniform_buffer)->buffer,
        0,
        ubo_size
    );
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;
    VK_CHECK(vkCreateFence(context->LogicDevice(), &fenceInfo, nullptr, &fence), "Can't create fence");

}

void VulkanCubeMapRender::shutdown()
{

    driver->destroyUniformBuffer(uniform_buffer);
    uniform_buffer = nullptr;

    driver->destroyFrameBuffer(framebuffer);
    framebuffer = nullptr;

    quad.clearGPUData();
    quad.clearCPUData();

    vkFreeCommandBuffers(context->LogicDevice(), context->CommandPool(), 1, &command_buffer);
    command_buffer = VK_NULL_HANDLE;

    vkDestroyDescriptorSetLayout(context->LogicDevice(), descriptor_set_layout, nullptr);
    descriptor_set_layout = VK_NULL_HANDLE;

    vkFreeDescriptorSets(context->LogicDevice(), context->DescriptorPool(), 1, &descriptorSet);
    descriptorSet = VK_NULL_HANDLE;

    vkDestroyRenderPass(context->LogicDevice(), render_pass, nullptr);
    render_pass = VK_NULL_HANDLE;

    vkDestroyPipelineLayout(context->LogicDevice(), pipeline_layout, nullptr);
    pipeline_layout = VK_NULL_HANDLE;

    vkDestroyPipeline(context->LogicDevice(), pipeline, nullptr);
    pipeline = VK_NULL_HANDLE;

    vkDestroyFence(context->LogicDevice(), fence, nullptr);
    fence = VK_NULL_HANDLE;

}

void VulkanCubeMapRender::render(const VulkanTexture &input_texture,
                                 float *userData,
                                 int input_mip)
{

    VkImageView mip_view = (input_mip == -1) ? VK_NULL_HANDLE : VulkanUtils::createImageView(
        context->LogicDevice(),
        input_texture.getImage(),
        input_texture.getImageFormat(),
        VK_IMAGE_ASPECT_COLOR_BIT,
        (input_texture.getNumLayers() == 6) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
        input_mip, 1, 0, input_texture.getNumLayers()
    );

    VulkanUtils::bindCombinedImageSampler(
        context->LogicDevice(),
        descriptorSet,
        1,
        (input_mip == -1) ? input_texture.getImageView() : mip_view,
        input_texture.getSampler()
    );

    // Record command buffers
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional
    VK_CHECK(vkBeginCommandBuffer(command_buffer, &beginInfo), "Can't begin recording command buffer");

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass;
    renderPassInfo.framebuffer = static_cast<render::backend::vulkan::FrameBuffer *>(framebuffer)->framebuffer;;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent.width = target_extent.width;
    renderPassInfo.renderArea.extent.height = target_extent.height;

    VkClearValue clearValues[6];
    for (int i = 0; i < 6; i++) {
        clearValues[i] = {};
        clearValues[i].color = {0.0f, 0.0f, 0.0f, 1.0f};
    }
    renderPassInfo.clearValueCount = 6;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(command_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout,
                            0,
                            1,
                            &descriptorSet,
                            0,
                            nullptr);
    if (push_constants_size > 0 && userData) {
        vkCmdPushConstants(command_buffer,
                           pipeline_layout,
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                           0,
                           push_constants_size,
                           userData);
    }

    VkBuffer vertexBuffers[] = {quad.getVertexBuffer()};
    VkBuffer indexBuffer = quad.getIndexBuffer();
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(command_buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(command_buffer, quad.getNumIndices(), 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);
    VK_CHECK(vkEndCommandBuffer(command_buffer), "Can't record command buffer");
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer;

    VK_CHECK(vkResetFences(context->LogicDevice(), 1, &fence), "Reset Fence Failed");
    VK_CHECK(vkQueueSubmit(context->GraphicsQueue(), 1, &submitInfo, fence), "Submit Queue Failed");
    VK_CHECK(vkWaitForFences(context->LogicDevice(), 1, &fence, VK_TRUE, 100000000000), "Can't wait for a fence");

}
}
