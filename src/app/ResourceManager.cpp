//
// Created by 12132 on 2021/10/30.
//

#include <iostream>
#include "ResourceManager.h"
#include "VulkanTexture.h"
#include "VulkanMesh.h"


VulkanResourceManager::VulkanResourceManager(const VulkanRenderContext &ctx) :context(ctx){

}

VulkanResourceManager::~VulkanResourceManager() {
    meshes.clear();
    textures.clear();
    shaders.clear();
}

std::shared_ptr<VulkanShader> VulkanResourceManager::loadShader(int id, ShaderKind kind, const char *path) {
    auto it = meshes.find(id);
    if(it !=meshes.end()){
        std::cerr << "VulkanResourceManager::loadShader():" <<id << "is alreay owned by other mesh" << std::endl;
        return nullptr;
    }
    auto shader = std::make_shared<VulkanShader>(context);
    shader->compileFromFile(path,kind);
    shaders.insert(std::make_pair( id,shader));
    return shader;
}

std::shared_ptr<VulkanMesh> VulkanResourceManager::loadMesh(int id, const char *path) {
    auto it = meshes.find(id);
    if(it !=meshes.end()){
        std::cerr << "VulkanResourceManager::loadMesh():" <<id << "is alreay owned by other " << std::endl;
        return nullptr;
    }
    auto mesh = std::make_shared<VulkanMesh>(context);
    mesh->loadFromFile(std::string(path));
    meshes.insert(std::make_pair( id,mesh));
    return mesh;
}

std::shared_ptr<VulkanTexture> VulkanResourceManager::loadTexture(int id, const char *path) {
    auto it = textures.find(id);
    if(it !=textures.end()){
        std::cerr << "VulkanResourceManager::loadTexture():" <<id << "is alreay owned by other mesh" << std::endl;
        return nullptr;
    }
    auto texture = std::make_shared<VulkanTexture>(context);
    texture->loadFromFile(path);
    textures.insert(std::make_pair( id,texture));
    return texture;
}

std::shared_ptr<VulkanShader> VulkanResourceManager::getShader(int id) {
    auto it = shaders.find(id);
    if (it !=shaders.end()){
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<VulkanTexture> VulkanResourceManager::getTexture(int id) {
    auto it = textures.find(id);
    if (it !=textures.end()){
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<VulkanMesh> VulkanResourceManager::getMesh(int id) {
    auto it = meshes.find(id);
    if (it !=meshes.end()){
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<VulkanTexture> VulkanResourceManager::loadHDRTexture(int id, const char *path) {
    auto it = textures.find(id);
    if(it !=textures.end()){
        std::cerr << "VulkanResourceManager::loadTexture():" <<id << "is alreay owned by other mesh" << std::endl;
        return nullptr;
    }
    auto texture = std::make_shared<VulkanTexture>(context);
    texture->loadHDRFromFile(path);
    textures.insert(std::make_pair( id,texture));
    return texture;
}

std::shared_ptr<VulkanMesh> VulkanResourceManager::createCubeMesh(int id, float size) {
    auto it = meshes.find(id);
    if(it !=meshes.end()){
        std::cerr << "VulkanResourceManager::loadMesh():" <<id << "is alreay owned by other " << std::endl;
        return nullptr;
    }
    auto mesh = std::make_shared<VulkanMesh>(context);
    mesh->createSkybox(size);
    meshes.insert(std::make_pair( id,mesh));
    return mesh;
}

std::shared_ptr<VulkanTexture> VulkanResourceManager::getHDRTexture(int id) {
    auto it = textures.find(id);
    if (it !=textures.end()){
        return it->second;
    }
    return nullptr;
}

