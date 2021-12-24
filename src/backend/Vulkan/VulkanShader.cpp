//
// Created by y123456 on 2021/10/17.
//

#include "VulkanShader.h"
#include "VulkanUtils.h"
#include <fstream>
#include <iostream>
#include <cstring>
namespace render::backend::vulkan {

VulkanShader::~VulkanShader() {
  clear();
}

bool VulkanShader::compileFromFile(const char *file_path,  render::backend::ShaderType shader_type) {

  std::ifstream file(file_path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "VulkanShader: Load Shader File Failed:" << file_path << "\n";
    return false;
  }
  clear();
  size_t fileSize = static_cast<uint32_t>(file.tellg());
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();
  shader = driver->createShaderFromSource(shader_type, static_cast<uint32_t>(buffer.size()), buffer.data(), file_path);

  path = std::string(file_path);
  type = shader_type;

  return shader != nullptr;
}

void VulkanShader::clear() {
  driver->destroyShader(shader);
  shader = nullptr;
}

bool VulkanShader::reload() {
  return compileFromFile(path.c_str(), type);
}

VkShaderModule VulkanShader::getShaderModule() const
{
    if (shader == nullptr)
        return VK_NULL_HANDLE;
    return static_cast<vulkan::Shader*>(shader)->shaderModule;
}

}