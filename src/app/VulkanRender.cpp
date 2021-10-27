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

static std::string commonCubeVertexShaderPath = "C:\\Users\\y123456\\Desktop\\Programming\\c_cpp\\GameEngine\\Resources\\shaders\\common.vert";
static std::string hdriToCubeFragmentShaderPath = "C:\\Users\\y123456\\Desktop\\Programming\\c_cpp\\GameEngine\\Resources\\shaders\\hdriToCube.frag";
static std::string diffuseIrradianceFragmentShaderPath = "C:\\Users\\y123456\\Desktop\\Programming\\c_cpp\\GameEngine\\Resources\\shaders\\diffuseIrrandiance.frag";

void VulkanRender::init(VulkanRenderScene* scene) {

    commonCubeVertexShader.compileFromFile(commonCubeVertexShaderPath, ShaderKind::vertex);
    hdriToCubeFragmentShader.compileFromFile(hdriToCubeFragmentShaderPath, ShaderKind::fragment);
    diffuseIrradianceFragmentShader.compileFromFile(diffuseIrradianceFragmentShaderPath, ShaderKind::fragment);

    environmentCubemap.createCube(VK_FORMAT_R32G32B32A32_SFLOAT, 256, 256, 1);
    diffuseIrradianceCubemap.createCube(VK_FORMAT_R32G32B32A32_SFLOAT, 256, 256, 1);

    {
        VulkanUtils::transitionImageLayout(
                context,
                environmentCubemap.getImage(),
                environmentCubemap.getImageFormat(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                0, environmentCubemap.getNumMiplevels(),
                0, environmentCubemap.getNumLayers()
        );

        hdriToCubeRenderer.init(
                commonCubeVertexShader,
                hdriToCubeFragmentShader,
                scene->getHDRTexture(),
                environmentCubemap
        );
        hdriToCubeRenderer.render();

        VulkanUtils::transitionImageLayout(
                context,
                environmentCubemap.getImage(),
                environmentCubemap.getImageFormat(),
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                0, environmentCubemap.getNumMiplevels(),
                0, environmentCubemap.getNumLayers()
        );
    }


    {
        VulkanUtils::transitionImageLayout(
                context,
                diffuseIrradianceCubemap.getImage(),
                diffuseIrradianceCubemap.getImageFormat(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                0, diffuseIrradianceCubemap.getNumMiplevels(),
                0, diffuseIrradianceCubemap.getNumLayers()
        );

        diffuseIrradianceRenderer.init(
                commonCubeVertexShader,
                diffuseIrradianceFragmentShader,
                environmentCubemap,
                diffuseIrradianceCubemap
        );
        diffuseIrradianceRenderer.render();

        VulkanUtils::transitionImageLayout(
                context,
                diffuseIrradianceCubemap.getImage(),
                diffuseIrradianceCubemap.getImageFormat(),
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                0, diffuseIrradianceCubemap.getNumMiplevels(),
                0, diffuseIrradianceCubemap.getNumLayers()
        );
    }


    const VulkanShader& vertShader = scene->getPBRVertexShader();
    const VulkanShader& fragShader = scene->getPBRFragmentShader();
    const VulkanShader &skyboxVertexShader = scene->getSkyboxVertexShader();
    const VulkanShader &skyboxFragmentShader = scene->getSkyboxFragmentShader();

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


    VulkanDescriptorSetLayoutBuilder descriptorSetLayoutBuilder(context);
    VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    descriptorSetLayout = descriptorSetLayoutBuilder
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
            .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage)
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

    VulkanPipelineLayoutBuilder skyboxPipelineLayoutBuilder(context);
    skyboxPipelineLayout = skyboxPipelineLayoutBuilder
            .addDescriptorSetLayout(descriptorSetLayout)
            .build();

    VulkanPipelineLayoutBuilder pipelineLayoutBuilder(context);
    pipelineLayoutBuilder.addDescriptorSetLayout(descriptorSetLayout);
    pbrPipelineLayout = pipelineLayoutBuilder.build();

    VulkanGraphicsPipelineBuilder pipelineBuilder(context,pbrPipelineLayout,renderPass);
    //VulkanGraphicsPipelineBuilder pipelineBuilder(context,descriptorSetLayout,renderPass);
    pipelineBuilder.addShaderStage(vertShader.getShaderModule(), VK_SHADER_STAGE_VERTEX_BIT);
    pipelineBuilder.addShaderStage(fragShader.getShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT);
    pipelineBuilder.addVertexInput(VulkanMesh::getVertexInputBindingDescription(),VulkanMesh::getAttributeDescriptions());
    pipelineBuilder.setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineBuilder.addViewport(viewport);
    pipelineBuilder.addScissor(scissor);
    pipelineBuilder.setRasterizerState(false, false, VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    pipelineBuilder.setMultisampleState(context.maxMSAASamples, true);
    pipelineBuilder.setDepthStencilState(true, true, VK_COMPARE_OP_LESS),
    pipelineBuilder.addBlendColorAttachment();
    pbrPipeline =  pipelineBuilder.build();

    VulkanGraphicsPipelineBuilder skyboxPipelineBuilder(context, pbrPipelineLayout, renderPass);
    skyboxPipeline = skyboxPipelineBuilder
            .addShaderStage(skyboxVertexShader.getShaderModule(), VK_SHADER_STAGE_VERTEX_BIT)
            .addShaderStage(skyboxFragmentShader.getShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT)
            .addVertexInput(VulkanMesh::getVertexInputBindingDescription(), VulkanMesh::getAttributeDescriptions())
            .setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .addViewport(viewport)
            .addScissor(scissor)
            .setRasterizerState(false, false, VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
            .setMultisampleState(context.maxMSAASamples, true)
            .setDepthStencilState(true, true, VK_COMPARE_OP_LESS)
            .addBlendColorAttachment()
            .build();

    // Create uniform buffers
    VkDeviceSize uboSize = sizeof(SharedRenderState);
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

    // Create descriptor sets
    std::vector<VkDescriptorSetLayout> layouts(imageCount,descriptorSetLayout);

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = context.descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = imageCount;
    descriptorSetAllocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(imageCount);
    if (vkAllocateDescriptorSets(context.device_, &descriptorSetAllocInfo, descriptorSets.data()) != VK_SUCCESS)
        throw std::runtime_error("Can't allocate descriptor sets");

    for (size_t i = 0; i < imageCount; i++)
    {
        std::array<const VulkanTexture *, 7> textures =
                {
                        &scene->getAlbedoTexture(),
                        &scene->getNormalTexture(),
                        &scene->getAOTexture(),
                        &scene->getShadingTexture(),
                        &scene->getEmissionTexture(),
                        &environmentCubemap,
                        &diffuseIrradianceCubemap,
                };

        VulkanUtils::bindUniformBuffer(
                context,
                descriptorSets[i],
                0,
                uniformBuffers[i],
                0,
                sizeof(SharedRenderState)
        );

        for (int k = 0; k < textures.size(); k++)
            VulkanUtils::bindCombinedImageSampler(
                    context,
                    descriptorSets[i],
                    k + 1,
                    textures[k]->getImageView(),
                    textures[k]->getSampler()
            );
    }

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

    if (vkAllocateCommandBuffers(context.device_, &allocateInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Can't create command buffers");

    // Record command buffers
    for (size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Can't begin recording command buffer");

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = frameBuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainContext.extend;

        std::array<VkClearValue, 3> clearValues = {};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[2].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline);
        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

        {
            const VulkanMesh &skybox = scene->getSkyboxMesh();

            VkBuffer vertexBuffers[] = { skybox.getVertexBuffer() };
            VkBuffer indexBuffer = skybox.getIndexBuffer();
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(commandBuffers[i], skybox.getNumIndices(), 1, 0, 0, 0);
        }

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipeline);
        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
        {
            const VulkanMesh &mesh = scene->getMesh();
            VkBuffer vertexBuffers[] = { mesh.getVertexBuffer() };
            VkBuffer indexBuffer = mesh.getIndexBuffer();
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(commandBuffers[i], mesh.getNumIndices(), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(commandBuffers[i]);
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Can't record command buffer");
    }
}


void VulkanRender::shutdown() {
    commonCubeVertexShader.clear();

    hdriToCubeFragmentShader.clear();
    hdriToCubeRenderer.shutdown();

    diffuseIrradianceFragmentShader.clear();
    diffuseIrradianceRenderer.shutdown();

    environmentCubemap.clearGPUData();
    diffuseIrradianceCubemap.clearGPUData();

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

    VK_DESTROY_OBJECT(vkDestroyDescriptorSetLayout(context.device_,descriptorSetLayout, nullptr),descriptorSetLayout);


    vkDestroyRenderPass(context.device_,renderPass, nullptr);
    renderPass = VK_NULL_HANDLE;

    vkDestroyPipelineLayout(context.device_,pbrPipelineLayout, nullptr);
    pbrPipelineLayout = VK_NULL_HANDLE;

    vkDestroyPipeline(context.device_,pbrPipeline, nullptr);
    pbrPipeline = VK_NULL_HANDLE;

    vkDestroyPipelineLayout(context.device_,skyboxPipelineLayout, nullptr);
    skyboxPipelineLayout = VK_NULL_HANDLE;

    vkDestroyPipeline(context.device_,skyboxPipeline, nullptr);
    skyboxPipeline = VK_NULL_HANDLE;


    descriptorSetLayout = VK_NULL_HANDLE;


}

VkCommandBuffer VulkanRender::render(uint32_t imageIndex) {


    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float,std::chrono::seconds::period>(currentTime-startTime).count();

    VkBuffer uniformBuffer = uniformBuffers[imageIndex];
    VkDeviceMemory uniformBufferMemory = uniformBuffersMemory[imageIndex];

    const glm::vec3 &up = {0.0f, 0.0f, 1.0f};
    const glm::vec3 &zero = {0.0f, 0.0f, 0.0f};
    const glm::vec3 &cameraPos = glm::vec3(2.0f, 2.0f, 2.0f);

    const float aspect = swapChainContext.extend.width / (float) swapChainContext.extend.height;
    const float zNear = 0.1f;
    const float zFar = 1000.0f;
    const float rotationSpeed = 0.1f;

    SharedRenderState *ubo = nullptr;
    vkMapMemory(context.device_, uniformBufferMemory, 0, sizeof(SharedRenderState), 0, reinterpret_cast<void**>(&ubo));



    ubo->world = glm::rotate(glm::mat4(1.0f), time * rotationSpeed * glm::radians(90.0f), up);
    ubo->view = glm::lookAt(cameraPos, zero, up);
    ubo->proj = glm::perspective(glm::radians(60.0f), aspect, zNear, zFar);
    ubo->proj[1][1] *= -1;
    ubo->cameraPos = cameraPos;

    vkUnmapMemory(context.device_, uniformBufferMemory);

    return commandBuffers[imageIndex];
}
