//
// Created by y123456 on 2021/10/12.
//


#include "VulkanRenderScene.h"
#include "Macro.h"

void VulkanRenderScene::init(const std::string& vertShaderFile,
                      const std::string& fragShaderFile,
                      const std::string& textureFile,
                      const std::string& modelFile){
    vertShader = createShader(vertShaderFile);
    fragShader = createShader(fragShaderFile);
    mesh.loadFromFile(modelFile);
    texture.loadFromFile(textureFile);
}

void VulkanRenderScene::shutdown(){
    mesh.clearGPUData();
    mesh.clearCPUData();
    texture.clearCPUData();
    texture.clearGPUData();
    vkDestroyShaderModule(context.device_,vertShader,nullptr);
    vkDestroyShaderModule(context.device_,fragShader,nullptr);
    vertShader = VK_NULL_HANDLE;
    fragShader = VK_NULL_HANDLE;
}

VkShaderModule VulkanRenderScene::createShader(const std::string &path) const {
    std::vector<char> vertex_code = vulkanUtils::readFile(path);
    VkShaderModuleCreateInfo shaderInfo={};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.pCode = reinterpret_cast<const uint32_t *>(vertex_code.data());
    shaderInfo.codeSize = static_cast<uint32_t> (vertex_code.size());
    VkShaderModule shader;
    VK_CHECK(vkCreateShaderModule(context.device_,&shaderInfo, nullptr,&shader),"Create shader module failed\n");
    return shader;
}
