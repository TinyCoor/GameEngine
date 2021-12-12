//
// Created by y123456 on 2021/10/20.
//

#ifndef GAMEENGINE_VULKANGRAPHICSPIPELINELAYOUTBUILDER_H
#define GAMEENGINE_VULKANGRAPHICSPIPELINELAYOUTBUILDER_H

#include <vector>
#include <volk.h>
#include "Device.h"
namespace render::backend::vulkan {
class VulkanPipelineLayoutBuilder {
private:

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{};
  std::vector<VkPushConstantRange> pushConstants;

public:
  VulkanPipelineLayoutBuilder(){}

  VulkanPipelineLayoutBuilder &addDescriptorSetLayout(
      VkDescriptorSetLayout descriptorSetLayout
  );

  VulkanPipelineLayoutBuilder &addPushConstantRange(
      VkShaderStageFlags stageFlags,
      uint32_t offset,
      uint32_t size
  );

  VkPipelineLayout build(VkDevice device);
};
}

#endif //GAMEENGINE_VULKANGRAPHICSPIPELINEBUILDER_H
