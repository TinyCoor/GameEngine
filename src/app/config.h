//
// Created by 12132 on 2021/10/30.
//

#ifndef GAMEENGINE_CONFIG_H
#define GAMEENGINE_CONFIG_H

#include <string>
#include <vector>
#include "VulkanShader.h"
namespace config{

    enum Shaders{
        PBRVertex,
        PBRFrag,
        SkyboxVertex,
        SkyboxFrag,
        CubeVertex,
        hdriToCubeFrag,
        diffuseIrrandianceFrag,
    };

    enum Textures{
        albedoTexture,
        normalTexture,
        aoTexture,
        shadingTexture,
        emissionTexture,
        Environment,
    };

    enum Meshes{
        SciFiHelmet,
        Skybox,
    };


    static std::vector< const char*> meshes={
            "model/SciFiHelmet.gltf",
    };


    static std::vector< const char*> shaders={
            "shaders/pbr.vert",
            "shaders/pbr.frag",
            "shaders/skybox.vert",
            "shaders/skybox.frag",
            "shaders/common.vert",
            "shaders/hdriToCube.frag",
            "shaders/diffuseIrrandiance.frag",
    };

    static std::vector< ShaderKind> shaderKinds={
            ShaderKind::vertex,
            ShaderKind::fragment,
            ShaderKind::vertex,
            ShaderKind::fragment,
            ShaderKind::vertex,
            ShaderKind::fragment,
            ShaderKind::fragment,
    };

    static std::vector< const char*> textures={
            "textures/SciFiHelmet_BaseColor.png",
            "textures/SciFiHelmet_Normal.png",
            "textures/SciFiHelmet_AmbientOcclusion.png",
            "textures/SciFiHelmet_MetallicRoughness.png",
            "textures/Default_emissive.jpg",
            "textures/environment/umbrellas.hdr",
    };

    static const char* hdrTexture= "textures/environment/umbrellas.hdr";
}



#endif //GAMEENGINE_CONFIG_H
