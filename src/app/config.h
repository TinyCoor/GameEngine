//
// Created by 12132 on 2021/10/30.
//

#ifndef GAMEENGINE_CONFIG_H
#define GAMEENGINE_CONFIG_H

#include <string>
#include <vector>
#include "VulkanShader.h"


//TODO read config from file
namespace config{

    enum Shaders{
        PBRVertex,
        PBRFrag,
        SkyboxVertex,
        SkyboxFrag,
        CubeVertex,
        hdriToCubeFrag,
        CubeToPrefilteredSpecular,
        diffuseIrrandianceFrag,
        BakedBRDFVertex,
        BakedBRDFFrag,
    };

    enum Textures{
        albedoTexture,
        normalTexture,
        aoTexture,
        shadingTexture,
        emissionTexture,
        EnvironmentBase,
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
            "../../assets/shaders/cubeToPrefilterSpecular.frag",
            "../../assets/shaders/diffuseIrrandiance.frag",
            "../../assets/shaders/bakedBRDF.vert",
            "../../assets/shaders/bakedBRDF.frag",
    };

    static std::vector< const char*> textures={
            "../../assets/textures/SciFiHelmet_BaseColor.png",
            "../../assets/textures/SciFiHelmet_Normal.png",
            "../../assets/textures/SciFiHelmet_AmbientOcclusion.png",
            "../../assets/textures/SciFiHelmet_MetallicRoughness.png",
            "../../assets/textures/Default_emissive.jpg",
    };

    static std::vector<const char*> hdrTextures= {
            "../../assets/textures/environment/arctic.hdr",
            "../../assets/textures/environment/umbrellas.hdr",
            "../../assets/textures/environment/shanghai_bund_4k.hdr"
    };

}



#endif //GAMEENGINE_CONFIG_H
