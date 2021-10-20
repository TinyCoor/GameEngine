//
// Created by y123456 on 2021/10/10.
//

#include "VulkanRender.h"
#include "VulkanGraphicsPipelineBuilder.h"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

#include "Macro.h"


void VulkanRender::init(VulkanRenderScene* scene) {

    size_t imageCount = swapChainContext.imageViews.size();
    //Creaet Uniform Buffers
    VkDeviceSize uboSize = sizeof(SharedRenderState);
    uniformBuffers.resize(imageCount);
    uniformBuffersMemory.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++) {
        vulkanUtils::createBuffer(context,
                                  uboSize,
                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  uniformBuffers[i],
                                  uniformBuffersMemory[i]);
    }

    const VulkanShader& vertShader = scene->getVertexShader();
    const VulkanShader& fragShader = scene->getFragmentShader();
    //
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo ={};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.module = vertShader.getShaderModule();
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.pName ="main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo ={};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.module =fragShader.getShaderModule();
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.pName ="main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo,fragShaderStageInfo};

    //Create vertex input
    auto bindingDescription = VulkanMesh::getVertexInputBindingDescription();
    auto attributes = VulkanMesh::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount =1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = attributes.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributes.data(); // Optional


    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

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

    //Creat View Port
    VkPipelineViewportStateCreateInfo viewportStateInfo{};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.pViewports = &viewport;
    viewportStateInfo.scissorCount = 1;
    viewportStateInfo.pScissors = &scissor;

    //光栅化
    VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
    rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerInfo.depthClampEnable = VK_FALSE;
    rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerInfo.lineWidth = 1.0f;
    rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerInfo.depthBiasEnable = VK_FALSE;
    rasterizerInfo.depthBiasConstantFactor = 0.0f; // Optional
    rasterizerInfo.depthBiasClamp = 0.0f; // Optional
    rasterizerInfo.depthBiasSlopeFactor = 0.0f; // Optional

    //Multisampling
    VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
    multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingInfo.sampleShadingEnable = VK_FALSE;
    multisamplingInfo.rasterizationSamples = context.maxMSAASamples;
    multisamplingInfo.minSampleShading = 1.0f; // Optional
    multisamplingInfo.pSampleMask = nullptr; // Optional
    multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
    multisamplingInfo.alphaToOneEnable = VK_FALSE; // Optional

    // Create Depth Stencil state
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.minDepthBounds = 0.0f; // Optional
    depthStencilInfo.maxDepthBounds = 1.0f; // Optional
    depthStencilInfo.stencilTestEnable = VK_FALSE;
    depthStencilInfo.front = {}; // Optional
    depthStencilInfo.back = {}; // Optional

    // Blend state
    VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo{};
    colorBlendAttachmentInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentInfo.blendEnable = VK_FALSE;
    colorBlendAttachmentInfo.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachmentInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachmentInfo.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachmentInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachmentInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachmentInfo.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
    colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingInfo.logicOpEnable = VK_FALSE;
    colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlendingInfo.attachmentCount = 1;
    colorBlendingInfo.pAttachments = &colorBlendAttachmentInfo;
    colorBlendingInfo.blendConstants[0] = 0.0f; // Optional
    colorBlendingInfo.blendConstants[1] = 0.0f; // Optional
    colorBlendingInfo.blendConstants[2] = 0.0f; // Optional
    colorBlendingInfo.blendConstants[3] = 0.0f; // Optional

    //Dynamic state
    VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
    };


    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = 2;
    dynamicStateInfo.pDynamicStates = dynamicStates;

    std::array<VkDescriptorSetLayoutBinding, 7> bindings;

    VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[0] = { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, stage, nullptr };

    for (uint32_t i = 1; i < 7; i++)
        bindings[i] = { i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, stage, nullptr };

    //Create DescriptorSetLayout
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(context.device_, &layoutInfo, nullptr, &descriptorSetLayout),"failed to create descriptor set layout!");

    // Create Descriptor Sets
    std::vector<VkDescriptorSetLayout> layouts(imageCount, descriptorSetLayout);
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = swapChainContext.descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = imageCount;
    descriptorSetAllocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(imageCount);
    VK_CHECK(vkAllocateDescriptorSets(context.device_, &descriptorSetAllocInfo, descriptorSets.data()),"failed to allocate descriptor sets!") ;

    for (size_t i = 0; i < imageCount; i++) {
        std::array<const VulkanTexture*, 6> textures = {
                &scene->getAlbedoTexture(),
                &scene->getNormalTexture(),
                &scene->getAOTexture(),
                &scene->getShadingTexture(),
                &scene->getEmissionTexture(),
                &scene->getHDRTexture()
        };

        vulkanUtils::bindUniformBuffer(
                context,
                descriptorSets[i],
                0,
                uniformBuffers[i],
                0,
                sizeof(SharedRenderState)
        );
        for (int j = 0; j < textures.size(); ++j) {
            vulkanUtils::bindCombinedImageSampler(
                    context,
                    descriptorSets[i],
                    j+1,
                    textures[j]->getImageView(),
                    textures[j]->getSampler()
            );
        }
    }

    //Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // Optional
    pipelineLayoutInfo.pSetLayouts =  &descriptorSetLayout; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional


    VK_CHECK(vkCreatePipelineLayout(context.device_, &pipelineLayoutInfo, nullptr, &pipelineLayout),"failed to create pipeline layout!");

    //create Vulkan Render Pass
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainContext.colorFormat;
    colorAttachment.samples = context.maxMSAASamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = swapChainContext.depthFormat;
    depthAttachment.samples = context.maxMSAASamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = swapChainContext.colorFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentResloveRef{};
    colorAttachmentResloveRef.attachment = 2;
    colorAttachmentResloveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResloveRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = {
            colorAttachment,
            depthAttachment,
            colorAttachmentResolve};
    //VulkanRender Render Pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(context.device_, &renderPassInfo, nullptr, &renderPass),"failed to create render pass!");

    //Create Graphics PipeLine
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportStateInfo;
    pipelineInfo.pRasterizationState = &rasterizerInfo;
    pipelineInfo.pMultisampleState = &multisamplingInfo;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.pColorBlendState = &colorBlendingInfo;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    VK_CHECK(vkCreateGraphicsPipelines(context.device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeLine),"failed to create graphics pipeline!");

    //Create FrameBuffer
    frameBuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++) {
      std::array<VkImageView,3> attachment = {
                swapChainContext.colorImageView,
                swapChainContext.depthImageView,
                swapChainContext.imageViews[i],


        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = attachment.size();
        framebufferInfo.pAttachments = attachment.data();
        framebufferInfo.width = swapChainContext.extend.width;
        framebufferInfo.height = swapChainContext.extend.height;
        framebufferInfo.layers = 1;

        VK_CHECK(vkCreateFramebuffer(context.device_, &framebufferInfo, nullptr, &frameBuffers[i]),"failed to create framebuffer!");
    }

    //create command buffers
    commandBuffers.resize(imageCount);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = context.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = commandBuffers.size();

    VK_CHECK(vkAllocateCommandBuffers(context.device_, &allocInfo, commandBuffers.data()),"failed to allocate command buffers!");

    //Record command buffers
    for (size_t i = 0; i <imageCount; i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr; // Optional

        VK_CHECK(vkBeginCommandBuffer(commandBuffers[i], &beginInfo), "failed to begin recording command buffer!");

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = frameBuffers[i];
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = swapChainContext.extend;


        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeLine);
        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

        const VulkanMesh& mesh = scene->getMesh();
        VkBuffer vertexBuffers[] = {mesh.getVertexBuffer()};
        VkBuffer indexBuffer = mesh.getIndexBuffer();
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffers[i], mesh.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(commandBuffers[i],static_cast<uint32_t>(mesh.getNumIndices()), 1, 0, 0, 0);
        vkCmdEndRenderPass(commandBuffers[i]);
        VK_CHECK(vkEndCommandBuffer(commandBuffers[i]), "failed to record command buffer!");
    }
}


void VulkanRender::shutdown() {

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

    VK_DESTROY_OBJECT(vkDestroyDescriptorSetLayout(context.device_,descriptorSetLayout, nullptr), descriptorSetLayout);

    vkDestroyPipeline(context.device_, graphicsPipeLine, nullptr);
    vkDestroyRenderPass(context.device_, renderPass, nullptr);
    vkDestroyPipelineLayout(context.device_,pipelineLayout, nullptr);

    descriptorSetLayout = VK_NULL_HANDLE;
    graphicsPipeLine = VK_NULL_HANDLE;
    renderPass = VK_NULL_HANDLE;
    pipelineLayout = VK_NULL_HANDLE;
}

VkCommandBuffer VulkanRender::render(uint32_t imageIndex) {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float,std::chrono::seconds::period>(currentTime-startTime).count();

    VkBuffer uniformBuffer = uniformBuffers[imageIndex];
    VkDeviceMemory uniformBufferMemory = uniformBuffersMemory[imageIndex];

    const glm::vec3& up = {0.f,0.f,1.f};
    const glm::vec3& zero = {0.f,0.f,0.f};

    const float aspect = swapChainContext.extend.width/(float)swapChainContext.extend.height;
    const float znear = 0.1f;
    const float zfar = 10.f;
   // float aspect = 1.0;

    SharedRenderState ubo{};
    ubo.world = glm::rotate(glm::mat4(1.0f), time * 0.1f * glm::radians(90.0f), up);
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), zero, up);
    ubo.proj = glm::perspective(glm::radians(45.0f), aspect, znear, zfar);
    ubo.cameraPos = glm::vec3(2.0f, 2.0f, 2.0f);
    ubo.proj[1][1] *= -1;

    void* data_uniform = nullptr;
    vkMapMemory(context.device_, uniformBufferMemory, 0, sizeof(ubo), 0, &data_uniform);
    memcpy(data_uniform, &ubo, sizeof(ubo));
    vkUnmapMemory(context.device_, uniformBufferMemory);

    return commandBuffers[imageIndex];
}
