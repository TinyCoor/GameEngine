//
// Created by y123456 on 2021/10/20.
//

#include "VulkanPipelineLayoutBuilder.h"
#include "VulkanUtils.h"
#include <stdexcept>


VulkanPipelineLayoutBuilder &
VulkanPipelineLayoutBuilder::addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) {
    descriptorSetLayouts.emplace_back(descriptorSetLayout);
    return *this;
}


VkPipelineLayout VulkanPipelineLayoutBuilder::build() {

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0; // TODO: add support for push constants
    pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();

    if (vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Can't create pipeline layout");

    return  pipelineLayout;

}

VulkanPipelineLayoutBuilder &
VulkanPipelineLayoutBuilder::addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size) {
    VkPushConstantRange range = {};
    range.stageFlags = stageFlags;
    range.offset = offset;
    range.size = size;

    pushConstants.push_back(range);
    return *this;

}

