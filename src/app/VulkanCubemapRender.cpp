//
// Created by y123456 on 2021/10/10.
//

#include "VulkanCubemapRender.h"
#include "VulkanGraphicsPipelineBuilder.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanPipelineLayoutBuilder.h"
#include "VulkanRenderPassBuilder.h"
#include "Macro.h"


const std::string renderCubemapShaderPath =  R"(C:\Users\y123456\Desktop\Programming\c_cpp\GameEngine\Resources\shaders\cubemap.frag)";
const std::string renderCubemapVertexShaderPath =  R"(C:\Users\y123456\Desktop\Programming\c_cpp\GameEngine\Resources\shaders\cubemap.vert)";


struct CubemapFaceOrientationData{
    glm::mat3 faces[6];
};

void VulkanCubeMapRender::init(const VulkanTexture& inputTexture,const VulkanTexture& targetTexture) {
    //  VulkanTexture* inputHDR= nullptr;
    renderVertexShader.compileFromFile(renderCubemapVertexShaderPath,ShaderKind::vertex);
    renderFragmentShader.compileFromFile(renderCubemapShaderPath,ShaderKind::fragment);
    renderQuad.createQuad(2.0f);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)targetTexture.getWidth();
    viewport.height = (float)targetTexture.getHeight();
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //create scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = targetTexture.getWidth();
    scissor.extent.height= targetTexture.getHeight();

    VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VulkanDescriptorSetLayoutBuilder descriptorSetLayoutBuilder(context);
    descriptorSetLayout = descriptorSetLayoutBuilder
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .build();


    VulkanRenderPassBuilder renderPassBuilder(context);
    renderPassBuilder.addColorAttachment(targetTexture.getImageFormat(),VK_SAMPLE_COUNT_1_BIT);
    renderPassBuilder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS);
    renderPassBuilder.addColorAttachmentReference(0,0);
    renderPass = renderPassBuilder.build();

    VulkanPipelineLayoutBuilder pipelineLayoutBuilder(context);
    pipelineLayoutBuilder.addDescriptorSetLayout(descriptorSetLayout);
    pbrPipelineLayout = pipelineLayoutBuilder.build();

    VulkanGraphicsPipelineBuilder pipelineBuilder(context,pbrPipelineLayout,renderPass);
    pipelineBuilder.addShaderStage(renderVertexShader.getShaderModule(), VK_SHADER_STAGE_VERTEX_BIT );
    pipelineBuilder.addShaderStage(renderFragmentShader.getShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT);
    pipelineBuilder.addVertexInput(VulkanMesh::getVertexInputBindingDescription(),VulkanMesh::getAttributeDescriptions());
    pipelineBuilder.setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineBuilder.addViewport(viewport);
    pipelineBuilder.addScissor(scissor);
    pipelineBuilder.setRasterizerState(false, false, VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    pipelineBuilder.setMultisampleState(VK_SAMPLE_COUNT_1_BIT);
    pipelineBuilder.setDepthStencilState(false, false, VK_COMPARE_OP_LESS),
    pipelineBuilder.addBlendColorAttachment();
    pbrPipeline =  pipelineBuilder.build();


    //TODO
    VkDeviceSize uboSize =sizeof(CubemapFaceOrientationData);

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
    VK_CHECK(vkAllocateDescriptorSets(context.device_, &descriptorSetAllocInfo, &descriptorSet),"Can't allocate descriptor sets");


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
            inputTexture.getImageView(),
            inputTexture.getSampler()
    );

    //create frameBuffer
    auto cubeimage = targetTexture.getImageView();
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &cubeimage;
    framebufferInfo.width = targetTexture.getWidth();
    framebufferInfo.height =targetTexture.getHeight();
    framebufferInfo.layers = targetTexture.getNumLayers();
    VK_CHECK(vkCreateFramebuffer(context.device_, &framebufferInfo, nullptr, &frameBuffer),"Can't create framebuffer");

    // Create command buffers
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = context.commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;
    VK_CHECK(vkAllocateCommandBuffers(context.device_, &allocateInfo, &commandBuffer),"Can't create command buffers");

    //fill uniform buffer
    CubemapFaceOrientationData* ubo{};
    vkMapMemory(context.device_, uniformBuffersMemory, 0, sizeof(CubemapFaceOrientationData), 0,reinterpret_cast<void **>(&ubo));
    for (int i = 0; i <6 ; ++i) {
        ubo->faces[i] = glm::mat3(1.0);
    }

    vkUnmapMemory(context.device_, uniformBuffersMemory);

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
    renderPassInfo.renderArea.extent.width = targetTexture.getWidth();
    renderPassInfo.renderArea.extent.height = targetTexture.getHeight();

    VkClearValue clearValue = {};
    clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues =&clearValue;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipelineLayout, 0, 1, &descriptorSet, 0, nullptr);


    VkBuffer vertexBuffers[] = {renderQuad.getVertexBuffer()};
    VkBuffer indexBuffer = renderQuad.getIndexBuffer();
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, renderQuad.getNumIndices(), 1, 0, 0, 0);

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
    renderFragmentShader.clear();
}

void VulkanCubeMapRender::render() {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount=1;
    submitInfo.pCommandBuffers =&commandBuffer;

    vkQueueSubmit(context.graphicsQueue,1,&submitInfo,VK_NULL_HANDLE);
    vkQueueWaitIdle(context.graphicsQueue);
}
