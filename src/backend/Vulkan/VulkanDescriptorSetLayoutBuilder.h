//
// Created by y123456 on 2021/10/21.
//

#ifndef GAMEENGINE_VULKANDESCRIPTORSETLAYOUTBUILDER_H
#define GAMEENGINE_VULKANDESCRIPTORSETLAYOUTBUILDER_H
#include "Device.h"
namespace render::backend::vulkan {
class VulkanDescriptorSetLayoutBuilder {
  std::vector<VkDescriptorSetLayoutBinding> bindings;
public:
  VulkanDescriptorSetLayoutBuilder() {}

  VulkanDescriptorSetLayoutBuilder &addDescriptorBinding(VkDescriptorType type,
                                                         VkShaderStageFlags shaderStageFlags,
                                                         uint32_t binding,
                                                         int descriptor_count = 1);

  VkDescriptorSetLayout build(VkDevice device);

};
}
#endif //GAMEENGINE_VULKANDESCRIPTORSETLAYOUTBUILDER_H
