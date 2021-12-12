//
// Created by y123456 on 2021/10/21.
//
#include <stdexcept>
#include "VulkanDescriptorSetLayoutBuilder.h"
namespace render::backend::vulkan {

VulkanDescriptorSetLayoutBuilder &
VulkanDescriptorSetLayoutBuilder::addDescriptorBinding(VkDescriptorType type,
                                                       VkShaderStageFlags shaderStageFlags,
                                                       uint32_t binding,
                                                       int descriptor_count
) {

  VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
  descriptorSetLayoutBinding.binding = binding;
  descriptorSetLayoutBinding.descriptorType = type;
  descriptorSetLayoutBinding.stageFlags = shaderStageFlags;
  descriptorSetLayoutBinding.descriptorCount = descriptor_count;
  bindings.emplace_back(descriptorSetLayoutBinding);

  return *this;
}

VkDescriptorSetLayout VulkanDescriptorSetLayoutBuilder::build(VkDevice device) {
  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
  descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  descriptorSetLayoutInfo.pBindings = bindings.data();

  VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
  if (vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout)
      != VK_SUCCESS){
      throw std::runtime_error("Can't create descriptor set layout");
  }

  return descriptorSetLayout;

}
}