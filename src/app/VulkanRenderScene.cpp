//
// Created by y123456 on 2021/10/12.
//


#include "VulkanRenderScene.h"

void VulkanRenderScene::init(const std::string& vertShaderFile,
                             const std::string& fragShaderFile,
                             const std::string& skyboxVertexShaderFile,
                             const std::string& skyboxFragmentShaderFIle,
                             const std::string& albedoFile,
                             const std::string& normalFile,
                             const std::string& aoFile,
                             const std::string& shadingFile,
                             const std::string &emissionFile,
                             const std::string& hdrFile,
                             const std::string& modelFile){
    pbrVertShader.compileFromFile(vertShaderFile,ShaderKind::vertex);
    pbrFragShader.compileFromFile(fragShaderFile,ShaderKind::fragment);
    skyboxVertexShader.compileFromFile(skyboxVertexShaderFile,ShaderKind::vertex);
    skyboxFragmentShader.compileFromFile(skyboxFragmentShaderFIle,ShaderKind::fragment);

    mesh.loadFromFile(modelFile);
    albedoTexture.loadFromFile(albedoFile);
    normalTexture.loadFromFile(normalFile);
    aoTexture.loadFromFile(aoFile);
    shadingTexture.loadFromFile(shadingFile);
    emissionTexture.loadFromFile(emissionFile);
    hdrTexture.loadHDRFromFile(hdrFile);

    skyboxMesh.createSkybox(100.0);


}

void VulkanRenderScene::shutdown(){
    mesh.clearGPUData();
    mesh.clearCPUData();

    skyboxMesh.clearCPUData();
    skyboxMesh.clearGPUData();

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

    pbrVertShader.clear();
    pbrFragShader.clear();

    skyboxVertexShader.clear();
    skyboxFragmentShader.clear();
}

