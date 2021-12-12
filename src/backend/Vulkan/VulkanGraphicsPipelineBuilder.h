//
// Created by y123456 on 2021/10/20.
//

#ifndef GAMEENGINE_VULKANGRAPHICSPIPELINEBUILDER_H
#define GAMEENGINE_VULKANGRAPHICSPIPELINEBUILDER_H

#include <vector>
#include <volk.h>
#include "Device.h"


namespace render::backend::vulkan {
class VulkanGraphicsPipelineBuilder {
private:
  VkRenderPass renderPass{};
  VkPipelineLayout pipelineLayout{};

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  std::vector<VkVertexInputBindingDescription> vertexInputBindings;
  std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;
  std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;

  std::vector<VkViewport> viewports{};
  std::vector<VkRect2D> scissors{};
  std::vector<VkDynamicState> dynamicStates{};

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
  VkPipelineRasterizationStateCreateInfo rasterizerState{};
  VkPipelineMultisampleStateCreateInfo multisamplingState{};
  VkPipelineDepthStencilStateCreateInfo depthStencilState{};

public:
  VulkanGraphicsPipelineBuilder(VkPipelineLayout pipelineLayout,
                                VkRenderPass renderPass)
      : pipelineLayout(pipelineLayout), renderPass(renderPass) {}

  inline VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }

  VulkanGraphicsPipelineBuilder &addShaderStage(
      VkShaderModule shader,
      VkShaderStageFlagBits stage,
      const char *entry = "main"
  );

  VulkanGraphicsPipelineBuilder &addVertexInput(
      const VkVertexInputBindingDescription &binding,
      const std::vector<VkVertexInputAttributeDescription> &attributes
  );

  VulkanGraphicsPipelineBuilder &addViewport(
      const VkViewport &viewport
  );

  VulkanGraphicsPipelineBuilder &addScissor(
      const VkRect2D &scissor
  );

  VulkanGraphicsPipelineBuilder &addDynamicState(VkDynamicState dynamicState);

  VulkanGraphicsPipelineBuilder &addBlendColorAttachment(
      bool blend = false,
      VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
      VkBlendOp colorBlendOp = VK_BLEND_OP_ADD,
      VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD,
      VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
          | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
  );

  VulkanGraphicsPipelineBuilder &setDepthStencilState(
      bool depthTest,
      bool depthWrite,
      VkCompareOp depthCompareOp
  );

  VulkanGraphicsPipelineBuilder &setInputAssemblyState(
      VkPrimitiveTopology topology,
      bool primitiveRestart = false
  );

  VulkanGraphicsPipelineBuilder &setRasterizerState(
      bool depthClamp,
      bool rasterizerDiscard,
      VkPolygonMode polygonMode,
      float lineWidth,
      VkCullModeFlags cullMode,
      VkFrontFace frontFace,
      bool depthBias = false,
      float depthBiasConstantFactor = 0.0f,
      float depthBiasClamp = 0.0f,
      float depthBiasSlopeFactor = 0.0f
  );

  VulkanGraphicsPipelineBuilder &setMultisampleState(
      VkSampleCountFlagBits msaaSamples,
      bool sampleShading = false,
      float minSampleShading = 1.0
  );

  VkPipeline build(VkDevice device);
};
}

#endif //GAMEENGINE_VULKANGRAPHICSPIPELINEBUILDER_H
