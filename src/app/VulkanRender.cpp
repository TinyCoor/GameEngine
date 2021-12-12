//
// Created by y123456 on 2021/10/10.
//

#include "VulkanRender.h"
#include "RenderState.h"
#include "../backend/Vulkan/VulkanSwapChain.h"
#include "../backend/Vulkan/VulkanGraphicsPipelineBuilder.h"
#include "../backend/Vulkan/VulkanDescriptorSetLayoutBuilder.h"
#include "../backend/Vulkan/VulkanPipelineLayoutBuilder.h"
#include "../backend/Vulkan/VulkanMesh.h"
#include "VulkanRenderScene.h"
#include "../backend/Vulkan/Macro.h"
#include "../backend/Vulkan/VulkanUtils.h"
#include "../backend/Vulkan/VulkanTexture.h"

#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

namespace render::backend::vulkan {
VulkanRender::VulkanRender(const Device *ctx,
                           render::backend::Driver *driver, VkExtent2D size, VkDescriptorSetLayout layout, VkRenderPass pass)
    : context(ctx),
      driver(driver),
      renderPass(pass),
      extent(size),
      descriptorSetLayout(layout),
      hdriToCubeRenderer(ctx,driver),
      diffuseIrradianceRenderer(ctx,driver),
      environmentCubemap(driver),
      diffuseIrradianceCubemap(driver),
      brdfBaked(driver),
      brdfRender(ctx,driver) {

}

VulkanRender::~VulkanRender() {
  shutdown();
}

void VulkanRender::init(VulkanRenderScene *scene) {

  const VulkanShader* vertShader = scene->getPBRVertexShader();
  const VulkanShader* fragShader = scene->getPBRFragmentShader();
  const VulkanShader* skyboxVertexShader = scene->getSkyboxVertexShader();
  const VulkanShader* skyboxFragmentShader = scene->getSkyboxFragmentShader();

  VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

  VulkanDescriptorSetLayoutBuilder sceneDescriptorSetLayoutBuilder;
  sceneDescriptorSetLayout = sceneDescriptorSetLayoutBuilder
      .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage,0)
      .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage,1)
      .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage,2)
      .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage,3)
      .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage,4)
      .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage,5)
      .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage,6)
      .addDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage,7)
      .build(context->LogicDevice());

  VulkanPipelineLayoutBuilder pipelineLayoutBuilder;
  pipelineLayoutBuilder.addDescriptorSetLayout(descriptorSetLayout);
  pipelineLayoutBuilder.addDescriptorSetLayout(sceneDescriptorSetLayout);
  pipelineLayout = pipelineLayoutBuilder.build(context->LogicDevice());

  VulkanGraphicsPipelineBuilder pbrpipelineBuilder(pipelineLayout, renderPass);
  pbrPipeline = pbrpipelineBuilder.addShaderStage(vertShader->getShaderModule(), VK_SHADER_STAGE_VERTEX_BIT)
      .addShaderStage(fragShader->getShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT)
      .addVertexInput(VulkanMesh::getVertexInputBindingDescription(), VulkanMesh::getAttributeDescriptions())
      .setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
      .addViewport(VkViewport())
      .addScissor(VkRect2D())
      .addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
      .addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
      .setRasterizerState(false, false, VK_POLYGON_MODE_FILL,
                          1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
      .setMultisampleState(context->getMaxSampleCount(), true)
      .setDepthStencilState(true, true, VK_COMPARE_OP_LESS)
      .addBlendColorAttachment()
      .build(context->LogicDevice());

  VulkanGraphicsPipelineBuilder skyboxPipelineBuilder(pipelineLayout, renderPass);
  skyboxPipeline = skyboxPipelineBuilder
      .addShaderStage(skyboxVertexShader->getShaderModule(), VK_SHADER_STAGE_VERTEX_BIT)
      .addShaderStage(skyboxFragmentShader->getShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT)
      .addVertexInput(VulkanMesh::getVertexInputBindingDescription(), VulkanMesh::getAttributeDescriptions())
      .setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
      .addViewport(VkViewport())
      .addScissor(VkRect2D())
      .addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
      .addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
      .setRasterizerState(false,
                          false,
                          VK_POLYGON_MODE_FILL,
                          1.0f,
                          VK_CULL_MODE_BACK_BIT,
                          VK_FRONT_FACE_COUNTER_CLOCKWISE)
      .setMultisampleState(context->getMaxSampleCount(), true)
      .setDepthStencilState(true, true, VK_COMPARE_OP_LESS)
      .addBlendColorAttachment()
      .build(context->LogicDevice());

  brdfBaked.create2D(render::backend::Format::R16G16_SFLOAT, 256, 256, 1);
  environmentCubemap.createCube(render::backend::Format::R32G32B32A32_SFLOAT, 256, 256, 1);
  diffuseIrradianceCubemap.createCube(render::backend::Format::R32G32B32A32_SFLOAT, 256, 256, 1);

  brdfRender.init(*scene->getBakedVertexShader(),
                  *scene->getBakedFragmentShader(),
                  brdfBaked);

  {
    VulkanUtils::transitionImageLayout(
        context,
        brdfBaked.getImage(),
        brdfBaked.getImageFormat(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        0, brdfBaked.getNumMiplevels(),
        0, brdfBaked.getNumLayers()
    );

    brdfRender.render();

    VulkanUtils::transitionImageLayout(
        context,
        brdfBaked.getImage(),
        brdfBaked.getImageFormat(),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        0, brdfBaked.getNumMiplevels(),
        0, brdfBaked.getNumLayers()
    );
  }

  hdriToCubeRenderer.init(
      *scene->getCubeVertexShader(),
      *scene->getHDRToCubeFragmentShader(),
      environmentCubemap,
      0
  );

  //TODO unique_ptr
  cubeToPrefilteredRenderers.resize(environmentCubemap.getNumMiplevels() - 1);
  for (int mip = 0; mip < environmentCubemap.getNumMiplevels() - 1; ++mip) {
    VulkanCubeMapRender *mipRenderer = new VulkanCubeMapRender(context,driver);
    mipRenderer->init(
       *scene->getCubeVertexShader(),
        *scene->getCubeToPrefilteredSpecularShader(),
        environmentCubemap,
        mip + 1);
    cubeToPrefilteredRenderers.emplace_back(mipRenderer);
  }

  diffuseIrradianceRenderer.init(
      *scene->getCubeVertexShader(),
      *scene->getDiffuseToIrridanceShader(),
      diffuseIrradianceCubemap,
      0
  );




  // Create scene descriptor sets
  VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
  descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descriptorSetAllocInfo.descriptorPool = context->DescriptorPool();
  descriptorSetAllocInfo.descriptorSetCount = 1;
  descriptorSetAllocInfo.pSetLayouts = &sceneDescriptorSetLayout;

  VK_CHECK(vkAllocateDescriptorSets(context->LogicDevice(), &descriptorSetAllocInfo, &sceneDescriptorSet),
           "Can't allocate descriptor sets");

  std::array<VulkanTexture*, 8> textures =
      {
          scene->getAlbedoTexture(),
          scene->getNormalTexture(),
          scene->getAOTexture(),
          scene->getShadingTexture(),
          scene->getEmissionTexture(),
          &environmentCubemap,
          &diffuseIrradianceCubemap,
          &brdfBaked,
      };

  for (int k = 0; k < textures.size(); k++)
    VulkanUtils::bindCombinedImageSampler(
        context->LogicDevice(),
        sceneDescriptorSet,
        k,
        textures[k]->getImageView(),
        textures[k]->getSampler()
    );
}

void VulkanRender::shutdown() {

  hdriToCubeRenderer.shutdown();
  diffuseIrradianceRenderer.shutdown();

  for (int i = 0; i < cubeToPrefilteredRenderers.size(); ++i) {
    cubeToPrefilteredRenderers[i]->shutdown();
    delete cubeToPrefilteredRenderers[i];
  }
  cubeToPrefilteredRenderers.clear();
  brdfRender.shutdown();

  brdfBaked.clearGPUData();
  brdfBaked.clearCPUData();

  environmentCubemap.clearGPUData();
  environmentCubemap.clearCPUData();

  diffuseIrradianceCubemap.clearGPUData();
  diffuseIrradianceCubemap.clearCPUData();

  vkFreeDescriptorSets(context->LogicDevice(), context->DescriptorPool(), 1, &sceneDescriptorSet);
  sceneDescriptorSet = VK_NULL_HANDLE;

  vkDestroyDescriptorSetLayout(context->LogicDevice(), sceneDescriptorSetLayout, nullptr);
  sceneDescriptorSetLayout = VK_NULL_HANDLE;

  vkDestroyPipelineLayout(context->LogicDevice(), pipelineLayout, nullptr);
  pipelineLayout = VK_NULL_HANDLE;

  vkDestroyPipeline(context->LogicDevice(), pbrPipeline, nullptr);
  pbrPipeline = VK_NULL_HANDLE;

  vkDestroyPipeline(context->LogicDevice(), skyboxPipeline, nullptr);
  skyboxPipeline = VK_NULL_HANDLE;

}

void VulkanRender::update(RenderState &state, VulkanRenderScene *scene) {
  // Render state
  static auto startTime = std::chrono::high_resolution_clock::now();
  auto currentTime = std::chrono::high_resolution_clock::now();

  const float rotationSpeed = 0.1f;
  float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

  const glm::vec3 &up = {0.0f, 0.0f, 1.0f};
  const glm::vec3 &zero = {0.0f, 0.0f, 0.0f};

  const float aspect = (float) extent.width / (float) extent.height;
  const float zNear = 0.1f;
  const float zFar = 1000.0f;

  const glm::vec3 &cameraPos = glm::vec3(2.0f, 2.0f, 2.0f);
  const glm::mat4 &rotation = glm::rotate(glm::mat4(1.0f), time * rotationSpeed * glm::radians(90.0f), up);

  state.world = glm::mat4(1.0f);
  state.view = glm::lookAt(cameraPos, zero, up) * rotation;
  state.proj = glm::perspective(glm::radians(60.0f), aspect, zNear, zFar);
  state.proj[1][1] *= -1;
  state.cameraPosWS = glm::vec3(glm::vec4(cameraPos, 1.0f) * rotation);

}

void VulkanRender::render(VulkanRenderScene *scene, const VulkanRenderFrame &frame) {
  VkCommandBuffer command_buffer = static_cast<vulkan::CommandBuffer*>(frame.command_buffer)->command_buffer;
  VkDescriptorSet descriptor_set = frame.descriptor_set;

  std::array<VkDescriptorSet, 2> sets = {descriptor_set, sceneDescriptorSet};

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
  scissor.extent = extent;

  vkCmdSetScissor(command_buffer, 0, 1, &scissor);
  vkCmdSetViewport(command_buffer, 0, 1, &viewport);


  vkCmdBindDescriptorSets(command_buffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout,
                          0,sets.size(),sets.data(),0,nullptr);
  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline);

  {
    auto skybox = scene->getSkyboxMesh();

    VkBuffer vertexBuffers[] = {skybox->getVertexBuffer()};
    VkBuffer indexBuffer = skybox->getIndexBuffer();
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(command_buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(command_buffer, skybox->getNumIndices(), 1, 0, 0, 0);
  }

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipeline);
  vkCmdBindDescriptorSets(command_buffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout,
                          0,sets.size(),sets.data(),0,nullptr);
  {
    auto mesh = scene->getMesh();

    VkBuffer vertexBuffers[] = {mesh->getVertexBuffer()};
    VkBuffer indexBuffer = mesh->getIndexBuffer();
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(command_buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(command_buffer, mesh->getNumIndices(), 1, 0, 0, 0);
  }
}

void VulkanRender::setEnvironment(VulkanTexture* texture) {

  {
    VulkanUtils::transitionImageLayout(
        context,
        environmentCubemap.getImage(),
        environmentCubemap.getImageFormat(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        0, 1,
        0, environmentCubemap.getNumLayers()
    );

    hdriToCubeRenderer.render(*texture);

    VulkanUtils::transitionImageLayout(
        context,
        environmentCubemap.getImage(),
        environmentCubemap.getImageFormat(),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        0, 1,
        0, environmentCubemap.getNumLayers()
    );
  }

  //mip
  for (size_t i = 0; i < cubeToPrefilteredRenderers.size(); ++i) {
    VulkanUtils::transitionImageLayout(
        context,
        environmentCubemap.getImage(),
        environmentCubemap.getImageFormat(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        i + 1, 1,
        0, environmentCubemap.getNumLayers()
    );
    float data[4] = {
        static_cast<float>(i) / environmentCubemap.getNumMiplevels(),
        0.0f, 0.0f, 0.0f
    };
    cubeToPrefilteredRenderers[i]->render(environmentCubemap, data, i);

    VulkanUtils::transitionImageLayout(
        context,
        environmentCubemap.getImage(),
        environmentCubemap.getImageFormat(),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        i + 1, 1,
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

    diffuseIrradianceRenderer.render(environmentCubemap);

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

  std::array<VulkanTexture*, 2> textures = {
      &environmentCubemap,
      &diffuseIrradianceCubemap,
  };

  for (int k = 0; k < textures.size(); k++)
    VulkanUtils::bindCombinedImageSampler(
        context->LogicDevice(),
        sceneDescriptorSet,
        k + 5,
        textures[k]->getImageView(),
        textures[k]->getSampler()
    );

}

void VulkanRender::resize(const VulkanSwapChain* swapChain) {
  extent = swapChain->getExtent();
}

void VulkanRender::reload(VulkanRenderScene *scene) {
  shutdown();
  init(scene);
}

}
