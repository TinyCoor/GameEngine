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

class VulkanShader {
public:
  explicit VulkanShader(render::backend::Driver *driver) : driver(driver) {}
  ~VulkanShader();

  bool compileFromFile(const char *path, render::backend::ShaderType type);

  bool reload();

  void clear();
  const VkShaderModule getShaderModule() const;

private:
  bool compileFromSource(const char *path, const char *source, size_t size, shaderc_shader_kind kind);

private:
  render::backend::Driver *driver {nullptr};
  render::backend::Shader *shader {nullptr};
  render::backend::ShaderType type {render::backend::ShaderType::FRAGMENT};

  std::string path;
};

}
#endif //GAMEENGINE_VULKANSHADER_H
