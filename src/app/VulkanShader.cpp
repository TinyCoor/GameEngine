//
// Created by y123456 on 2021/10/17.
//

#include "VulkanShader.h"
#include "VulkanUtils.h"
#include <fstream>
#include <iostream>


bool VulkanShader::loadFromFile(const std::string &path) {
    std::ifstream  file(path,std::ios::ate | std::ios::binary);
    if(!file.is_open()){
        std::cerr << "VulkanShader: Laod Shder File Failed:"<< path << "\n";
        return  false;
    }
    clear();
    size_t fileSize = static_cast<uint32_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(),fileSize);
    file.close();
    //TODO GLSL to spir byte code

    clear();
    shaderModule = vulkanUtils::createShaderModule(context,reinterpret_cast<uint32_t*>(buffer.data()),buffer.size());
    return true;
}

void VulkanShader::clear() {
    vkDestroyShaderModule(context.device_,shaderModule, nullptr);
    shaderModule = VK_NULL_HANDLE;
}
