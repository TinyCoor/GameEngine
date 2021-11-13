//
// Created by y123456 on 2021/10/12.
//


#include "VulkanRenderScene.h"

VulkanRenderScene::VulkanRenderScene(const VulkanContext* ctx)
: resource(ctx)
{}

VulkanRenderScene::~VulkanRenderScene() {
    shutdown();
}

void VulkanRenderScene::init(){
    for(int i =0; i< config::shaders.size();++i){
        resource.loadShader(i,config::shaders[i]);
    }
    for(int i =0; i< config::textures.size();++i){
        resource.loadTexture(i,config::textures[i]);
    }

    for(int i =0; i< config::meshes.size();++i){
        resource.loadMesh(i,config::meshes[i]);
    }

    for (int i = 0; i <config::hdrTextures.size() ; ++i) {
        resource.loadTexture(config::Textures::EnvironmentBase +i,config::hdrTextures[i]);
    }

    resource.createCubeMesh(config::Meshes::Skybox,1000.0);

}


void VulkanRenderScene::shutdown(){
    resource.shutdown();
}

bool  VulkanRenderScene::reloadShader() {
    for (int i = 0; i < config::shaders.size() ; ++i) {
        if( resource.reloadShader(i)){
            return true;
        }
    }
    return false;
}



