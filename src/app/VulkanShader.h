//
// Created by y123456 on 2021/10/17.
//

#ifndef GAMEENGINE_VULKANSHADER_H
#define GAMEENGINE_VULKANSHADER_H

#include "VulkanRenderContext.h"
#include <volk.h>
#include <stdexcept>
#include <shaderc/shaderc.h>


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
    ~VulkanShader();

    bool compileFromFile(const char* path,ShaderKind kind);
    bool compileFromFile(const char* path);


    void clear();
    inline const VkShaderModule& getShaderModule() const {return shaderModule;}

private:
    bool compileFromSource(const char* path,const char* source,size_t size,shaderc_shader_kind kind);

private:
    VulkanRenderContext context;
    VkShaderModule shaderModule{VK_NULL_HANDLE};
};


#endif //GAMEENGINE_VULKANSHADER_H
