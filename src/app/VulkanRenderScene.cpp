//
// Created by y123456 on 2021/10/12.
//


#include "VulkanRenderScene.h"
#include "config.h"
#include <vector>


VulkanRenderScene::VulkanRenderScene(VulkanRenderContext& ctx)
:context(ctx), resource(context)
{
}

void VulkanRenderScene::init(){
    for(int i =0; i< config::shaders.size();++i){
        resource.loadShader(i,config::shaderKinds[i],config::shaders[i]);

    }
    for(int i =0; i< config::textures.size();++i){
        resource.loadTexture(i,config::textures[i]);
    }
    resource.loadHDRTexture(config::Textures::Environment,config::hdrTexture);
    for(int i =0; i< config::meshes.size();++i){
        resource.loadMesh(i,config::meshes[i]);
    }
    resource.createCubeMesh(config::Meshes::Skybox,2.0);


}


void VulkanRenderScene::shutdown(){
//TODO Cleanup
}

