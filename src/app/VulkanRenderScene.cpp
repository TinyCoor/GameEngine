//
// Created by y123456 on 2021/10/12.
//


#include "VulkanRenderScene.h"

void VulkanRenderScene::init(const std::string& vertShaderFile,
                             const std::string& fragShaderFile,
                             const std::string& albedoFile,
                             const std::string& normalFile,
                             const std::string& aoFile,
                             const std::string& shadingFile,
                             const std::string &emissionFile,
                             const std::string& hdrFile,
                             const std::string& modelFile){
    vertShader.compileFromFile(vertShaderFile,ShaderKind::vertex);
    fragShader.compileFromFile(fragShaderFile,ShaderKind::fragment);
    mesh.loadFromFile(modelFile);
    albedoTexture.loadFromFile(albedoFile);
    normalTexture.loadFromFile(normalFile);
    aoTexture.loadFromFile(aoFile);
    shadingTexture.loadFromFile(shadingFile);
    emissionTexture.loadFromFile(emissionFile);
    hdrTexture.loadHDRFromFile(hdrFile);

}

void VulkanRenderScene::shutdown(){
    mesh.clearGPUData();
    mesh.clearCPUData();

    albedoTexture.clearCPUData();
    albedoTexture.clearGPUData();
    normalTexture.clearCPUData();
    normalTexture.clearGPUData();
    aoTexture.clearCPUData();
    aoTexture.clearGPUData();
    shadingTexture.clearCPUData();
    shadingTexture.clearGPUData();
    emissionTexture.clearGPUData();
    emissionTexture.clearCPUData();

    hdrTexture.clearCPUData();
    hdrTexture.clearGPUData();

    fragShader.clear();
    vertShader.clear();
}

