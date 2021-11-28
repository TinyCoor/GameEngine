//
// Created by 12132 on 2021/10/30.
//

#include <iostream>
#include "VulkanResourceManager.h"
#include "VulkanTexture.h"
#include "VulkanMesh.h"
using namespace render::backend::vulkan;


VulkanResourceManager::~VulkanResourceManager() {
    shutdown();
}

VulkanShader* VulkanResourceManager::loadShader(int id,render::backend::ShaderType type, const char *path) {
    auto it = shaders.find(id);
    if(it !=shaders.end()){
        std::cerr << "VulkanResourceManager::loadShader():" <<id << "is already owned by other mesh" << std::endl;
        return nullptr;
    }
    auto shader =new VulkanShader(driver);
    shader->compileFromFile(path,type);
    shaders.insert(std::make_pair( id,shader));
    return shader;
}



VulkanMesh* VulkanResourceManager::loadMesh(int id, const char *path) {
    auto it = meshes.find(id);
    if(it !=meshes.end()){
        std::cerr << "VulkanResourceManager::loadMesh():" <<id << "is already owned by other " << std::endl;
        return nullptr;
    }
    auto mesh = new VulkanMesh(driver);
    mesh->loadFromFile(path);
    meshes.insert(std::make_pair( id,mesh));
    return mesh;
}

VulkanTexture* VulkanResourceManager::loadTexture(int id, const char *path) {
    auto it = textures.find(id);
    if(it !=textures.end()){
        std::cerr << "VulkanResourceManager::loadTexture():" <<id << "is alreay owned by other mesh" << std::endl;
        return nullptr;
    }
    auto texture = new VulkanTexture(driver);
    texture->loadFromFile(path);
    textures.insert(std::make_pair( id,texture));
    return texture;
}

VulkanShader* VulkanResourceManager::getShader(int id) const {
    auto it = shaders.find(id);
    if (it !=shaders.end()){
        return it->second;
    }
    return nullptr;
}

VulkanTexture* VulkanResourceManager::getTexture(int id) const{
    auto it = textures.find(id);
    if (it !=textures.end()){
        return it->second;
    }
    return nullptr;
}

VulkanMesh* VulkanResourceManager::getMesh(int id) const {
    auto it = meshes.find(id);
    if (it !=meshes.end()){
        return it->second;
    }
    return nullptr;
}


VulkanMesh* VulkanResourceManager::createCubeMesh(int id, float size) {
    auto it = meshes.find(id);
    if(it !=meshes.end()){
        std::cerr << "VulkanResourceManager::loadMesh():" <<id << "is alreay owned by other " << std::endl;
        return nullptr;
    }
    auto mesh = new VulkanMesh(driver);
    mesh->createSkybox(size);
    meshes.insert(std::make_pair( id,mesh));
    return mesh;
}

VulkanTexture* VulkanResourceManager::getHDRTexture(int id) const {
    auto it = textures.find(id);
    if (it !=textures.end()){
        return it->second;
    }
    return nullptr;
}

bool VulkanResourceManager::reloadShader(int id) {
    auto it = shaders.find(id);
    if(it != shaders.end()){
        if(it->second->reload()){
            return false;
        }
    }
    return true;
}

void VulkanResourceManager::shutdown() {
    for (auto& texture:textures) {
        texture.second->clearGPUData();
        texture.second->clearCPUData();
    }

    for (auto& mesh:meshes) {
        mesh.second->clearGPUData();
        mesh.second->clearCPUData();
    }

    for (auto& shader : shaders) {
        shader.second->clear();
    }

    meshes.clear();
    textures.clear();
    shaders.clear();
}

