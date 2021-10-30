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
            "../../assets/models/SciFiHelmet.gltf",
    };


    static std::vector< const char*> shaders={
            "../../assets/shaders/pbr.vert",
            "../../assets/shaders/pbr.frag",
            "../../assets/shaders/skybox.vert",
            "../../assets/shaders/skybox.frag",
            "../../assets/shaders/common.vert",
            "../../assets/shaders/hdriToCube.frag",
            "../../assets/shaders/diffuseIrrandiance.frag",
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
            "../../assets/textures/SciFiHelmet_BaseColor.png",
            "../../assets/textures/SciFiHelmet_Normal.png",
            "../../assets/textures/SciFiHelmet_AmbientOcclusion.png",
            "../../assets/textures/SciFiHelmet_MetallicRoughness.png",
            "../../assets/textures/Default_emissive.jpg",
    };

    static const char* hdrTexture= "../../assets/textures/environment/umbrellas.hdr";
}



#endif //GAMEENGINE_CONFIG_H
