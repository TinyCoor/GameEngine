//
// Created by y123456 on 2021/10/17.
//

#ifndef GAMEENGINE_VULKANSHADER_H
#define GAMEENGINE_VULKANSHADER_H

#include "VulkanContext.h"
#include <volk.h>
#include <shaderc/shaderc.h>
#include <string_view>

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
    explicit VulkanShader(const VulkanContext* ctx):context(ctx){}
    ~VulkanShader();

    bool compileFromFile(const char* path,ShaderKind kind);
    bool compileFromFile(const char* path);

    bool reload();

    void clear();
    inline const VkShaderModule& getShaderModule() const {return shaderModule;}

private:
    bool compileFromSource(const char* path,const char* source,size_t size,shaderc_shader_kind kind);

private:
    const VulkanContext* context;
    VkShaderModule shaderModule{VK_NULL_HANDLE};
    std::string_view shader_path;
};


#endif //GAMEENGINE_VULKANSHADER_H
