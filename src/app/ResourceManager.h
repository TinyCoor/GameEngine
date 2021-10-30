//
// Created by 12132 on 2021/10/30.
//

#ifndef GAMEENGINE_RESOURCEMANAGER_H
#define GAMEENGINE_RESOURCEMANAGER_H
#include "VulkanRenderContext.h"
#include "VulkanShader.h"
#include <unordered_map>
#include <memory>

class VulkanShader;
class VulkanTexture;
class VulkanMesh;

//TODO Make User defined Deconstructor
class VulkanResourceManager {

public:
    VulkanResourceManager(const VulkanRenderContext& ctx);
    ~VulkanResourceManager();

    std::shared_ptr<VulkanShader> loadShader(int id, ShaderKind kind,const char* path);
    std::shared_ptr<VulkanMesh> loadMesh(int id,const char* path);
    std::shared_ptr<VulkanMesh> createCubeMesh(int id,float size);
    std::shared_ptr<VulkanTexture>  loadTexture(int id,const char* path);
    std::shared_ptr<VulkanTexture>  loadHDRTexture(int id,const char* path);

    std::shared_ptr<VulkanShader> getShader(int id);
    std::shared_ptr<VulkanTexture>  getTexture(int id);
    std::shared_ptr<VulkanMesh> getMesh(int id);
    std::shared_ptr<VulkanTexture> getHDRTexture(int id);

private:
    //May be This is singleton
    VulkanRenderContext context;
    std::unordered_map<int,std::shared_ptr<VulkanShader>> shaders;
    std::unordered_map<int,std::shared_ptr<VulkanMesh>> meshes;
    std::unordered_map<int,std::shared_ptr<VulkanTexture>> textures;
    std::shared_ptr<VulkanTexture> hdrTexture;
};


#endif //GAMEENGINE_RESOURCEMANAGER_H
