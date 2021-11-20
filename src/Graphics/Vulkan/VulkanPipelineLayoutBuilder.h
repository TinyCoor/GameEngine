//
// Created by y123456 on 2021/10/20.
//

#ifndef GAMEENGINE_VULKANGRAPHICSPIPELINELAYOUTBUILDER_H
#define GAMEENGINE_VULKANGRAPHICSPIPELINELAYOUTBUILDER_H

#include <vector>
#include <volk.h>
#include "VulkanContext.h"

class VulkanPipelineLayoutBuilder {
private:
    const VulkanContext* context;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{};
    VkPipelineLayout  pipelineLayout{};

    std::vector<VkPushConstantRange> pushConstants;

public:
    VulkanPipelineLayoutBuilder(const VulkanContext* ctx)
    : context(ctx){}

    inline VkPipelineLayout  getPipelineLayout() const {return pipelineLayout;}

    VulkanPipelineLayoutBuilder &addDescriptorSetLayout(
            VkDescriptorSetLayout descriptorSetLayout
            );

    VulkanPipelineLayoutBuilder &addPushConstantRange(
            VkShaderStageFlags stageFlags,
            uint32_t offset,
            uint32_t size
    );

    VkPipelineLayout build();
};


#endif //GAMEENGINE_VULKANGRAPHICSPIPELINEBUILDER_H
