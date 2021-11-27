//
// Created by y123456 on 2021/10/21.
//

#ifndef GAMEENGINE_VULKANDESCRIPTORSETLAYOUTBUILDER_H
#define GAMEENGINE_VULKANDESCRIPTORSETLAYOUTBUILDER_H
#include "VulkanContext.h"
namespace render::backend::vulkan {
class VulkanDescriptorSetLayoutBuilder {
  const VulkanContext *context;
  VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
  std::vector<VkDescriptorSetLayoutBinding> bindings;
public:
  VulkanDescriptorSetLayoutBuilder(const VulkanContext *ctx) : context(ctx) {
  }

  VulkanDescriptorSetLayoutBuilder &addDescriptorBinding(VkDescriptorType type,
                                                         VkShaderStageFlags shaderStageFlags);

  inline VkDescriptorSetLayout &getDescriptorSetLayout() { return descriptorSetLayout; }

  VkDescriptorSetLayout build();

};
}
#endif //GAMEENGINE_VULKANDESCRIPTORSETLAYOUTBUILDER_H
