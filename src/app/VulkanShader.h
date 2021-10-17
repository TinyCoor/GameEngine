//
// Created by y123456 on 2021/10/17.
//

#ifndef GAMEENGINE_VULKANSHADER_H
#define GAMEENGINE_VULKANSHADER_H

#include "VulkanRenderContext.h"
#include <volk.h>
#include <string>

class VulkanShader {
public:
    explicit VulkanShader(const VulkanRenderContext& ctx): context(ctx){}

    bool loadFromFile(const std::string& path);

    void clear();
    inline const VkShaderModule& getShaderModule() const {return shaderModule;}

private:
    VulkanRenderContext context;
    VkShaderModule shaderModule{VK_NULL_HANDLE};
};


#endif //GAMEENGINE_VULKANSHADER_H
