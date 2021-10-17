//
// Created by y123456 on 2021/10/17.
//

#ifndef GAMEENGINE_VULKANSHADER_H
#define GAMEENGINE_VULKANSHADER_H

#include "VulkanRenderContext.h"
#include <volk.h>
#include <string>
#include <stdexcept>


enum class ShaderKind{
    vertex =0,
    fragment,
    compute,
    geometry,
    tessellation_control,
    tessellation_evaluation,

};




class VulkanShader {
public:
    explicit VulkanShader(const VulkanRenderContext& ctx): context(ctx){}

    bool compileFromFile(const std::string& path,ShaderKind kind);

    void clear();
    inline const VkShaderModule& getShaderModule() const {return shaderModule;}

private:


private:
    VulkanRenderContext context;
    VkShaderModule shaderModule{VK_NULL_HANDLE};
};


#endif //GAMEENGINE_VULKANSHADER_H
