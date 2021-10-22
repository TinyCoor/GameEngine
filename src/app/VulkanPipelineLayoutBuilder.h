//
// Created by y123456 on 2021/10/20.
//

#ifndef GAMEENGINE_VULKANGRAPHICSPIPELINELAYOUTBUILDER_H
#define GAMEENGINE_VULKANGRAPHICSPIPELINELAYOUTBUILDER_H

#include <vector>
#include <volk.h>
#include "VulkanRenderContext.h"

class VulkanPipelineLayoutBuilder {
private:
    VulkanRenderContext context;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{};
    VkPipelineLayout  pipelineLayout{};

public:
    VulkanPipelineLayoutBuilder(const VulkanRenderContext& ctx)
    : context(ctx){}

    inline VkPipelineLayout  getPipelineLayout() const {return pipelineLayout;}

    VulkanPipelineLayoutBuilder &addDescriptorSetLayout(
            VkDescriptorSetLayout descriptorSetLayout
            );

    VkPipelineLayout build();
};


#endif //GAMEENGINE_VULKANGRAPHICSPIPELINEBUILDER_H
