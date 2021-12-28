//
// Created by y123456 on 2021/10/17.
//

#ifndef GAMEENGINE_VULKANSHADER_H
#define GAMEENGINE_VULKANSHADER_H

#include "driver.h"
#include <volk.h>
#include <shaderc/shaderc.h>
#include <string_view>
#include <string>
namespace render::backend::vulkan {
enum class ShaderKind {
    vertex = 0,
    fragment,
    compute,
    geometry,
    tessellation_control,
    tessellation_evaluation,
};

struct Shader : public render::backend::Shader {
    ShaderType type{ShaderType::FRAGMENT};
    VkShaderModule shaderModule{VK_NULL_HANDLE};
};

class VulkanShader {
public:
    explicit VulkanShader(render::backend::Driver *driver) : driver(driver)
    {}
    ~VulkanShader();

    bool compileFromFile(const char *path, render::backend::ShaderType type);
    bool reload();
    void clear();
    render::backend::Shader *getShader() const { return shader; }
    VkShaderModule getShaderModule() const;

private:
    render::backend::Driver *driver{nullptr};
    render::backend::Shader *shader{nullptr};
    render::backend::ShaderType type{render::backend::ShaderType::FRAGMENT};

    std::string path;
};

}
#endif //GAMEENGINE_VULKANSHADER_H
