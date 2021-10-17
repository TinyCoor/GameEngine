//
// Created by y123456 on 2021/10/12.
//


#include "VulkanRenderScene.h"

void VulkanRenderScene::init(const std::string& vertShaderFile,
                      const std::string& fragShaderFile,
                      const std::string& textureFile,
                      const std::string& modelFile){
    vertShader.loadFromFile(vertShaderFile);
    fragShader.loadFromFile(fragShaderFile);
    mesh.loadFromFile(modelFile);
    texture.loadFromFile(textureFile);
}

void VulkanRenderScene::shutdown(){
    mesh.clearGPUData();
    mesh.clearCPUData();
    texture.clearCPUData();
    texture.clearGPUData();
    fragShader.clear();
    vertShader.clear();
}

