//
// Created by y123456 on 2021/10/12.
//

#ifndef GAMEENGINE_RENDER_DATA_H
#define GAMEENGINE_RENDER_DATA_H
#include "VulkanUtils.h"
#include "VulkanRenderContext.h"
#include "VulkanMesh.h"
#include "VulkanTexture.h"
#include "volk.h"
#include <vector>
#include <string>
#include "VulkanShader.h"


class VulkanRenderScene {
private:

    VulkanRenderContext context;
    VulkanMesh mesh;
    VulkanMesh skyboxMesh;

   // std::vector<std::string> textures;
    VulkanTexture albedoTexture;
    VulkanTexture normalTexture;
    VulkanTexture aoTexture;
    VulkanTexture shadingTexture;
    VulkanTexture emissionTexture;
    VulkanTexture hdrTexture;
    VulkanShader pbrVertShader;
    VulkanShader pbrFragShader;
    VulkanShader skyboxVertexShader;
    VulkanShader skyboxFragmentShader;
public:
   explicit VulkanRenderScene(VulkanRenderContext& ctx)
        :context(ctx), mesh(ctx), skyboxMesh(ctx),
        albedoTexture(ctx),
        normalTexture(context),
        aoTexture(context),
        shadingTexture(ctx),
        emissionTexture(ctx),
        hdrTexture(ctx),
        pbrVertShader(context),
        pbrFragShader(context), skyboxVertexShader(context),
        skyboxFragmentShader(context)
        {}


     void init(const std::string& pbrVertShaderFile,
              const std::string& pbrFragShaderFile,
              const std::string& skyboxVertShaderFile,
              const std::string& skyboxFragmentShaderFile,
              const std::string& albedoFile,
              const std::string& normalFile,
              const std::string& aoFile,
              const std::string& shadingFile,
              const std::string& emissionFile,
              const std::string& hdrFile,
              const std::string& modelFile);

     inline const VulkanShader& getPBRVertexShader(){return pbrVertShader;}
     inline const VulkanShader& getPBRFragmentShader() {return pbrFragShader;}
     inline const VulkanShader& getSkyboxVertexShader(){return skyboxVertexShader;}
     inline const VulkanShader& getSkyboxFragmentShader() {return skyboxFragmentShader;}
     inline const VulkanTexture &getEmissionTexture() const { return emissionTexture; }
     inline const VulkanTexture &getAlbedoTexture() const { return albedoTexture; }
     inline const VulkanTexture& getHDRTexture()const{return  hdrTexture;}
     inline const VulkanTexture &getNormalTexture() const { return normalTexture; }
     inline const VulkanTexture &getAOTexture() const { return aoTexture; }
     inline const VulkanTexture &getShadingTexture() const { return shadingTexture; }

    inline const VulkanMesh& getMesh() const {return mesh;}
    inline const VulkanMesh& getSkyboxMesh() const{return skyboxMesh;}


     void shutdown() ;

};

#endif //GAMEENGINE_RENDER_DATA_H
