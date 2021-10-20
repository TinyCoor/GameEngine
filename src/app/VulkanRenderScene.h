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

class IModelLoader{
public:
    virtual VulkanMesh loadModel(const std::string& path) =0;
};



class RenderMaterial{
private:


};

//class RenderScene{
//public:
//    virtual void init(const std::string& vertShaderFile,
//                      const std::string& fragShaderFile,
//                      const std::string& textureFile,
//                      const std::string& modelFile)=0;
//    virtual VkShaderModule getVertexShader()  =0;
//    virtual VkShaderModule getFragmentShader()  =0;
//    virtual VulkanTexture getTexture() =0;
//    virtual VulkanMesh getMesh() =0;
//    virtual void shutdown() = 0;
//};

class VulkanRenderScene {
private:
    VulkanRenderContext context;
    VulkanMesh mesh;
   // std::vector<std::string> textures;
    VulkanTexture albedoTexture;
    VulkanTexture normalTexture;
    VulkanTexture aoTexture;
    VulkanTexture shadingTexture;
    VulkanTexture emissionTexture;
    VulkanTexture hdrTexture;
    VulkanShader vertShader;
    VulkanShader fragShader;
public:
   explicit VulkanRenderScene(VulkanRenderContext& ctx)
        :context(ctx), mesh(ctx),
        albedoTexture(ctx),
        normalTexture(context),
        aoTexture(context),
        shadingTexture(ctx),
        emissionTexture(ctx),
         hdrTexture(ctx),
        vertShader(context),
        fragShader(context){
   }


     void init(const std::string& vertShaderFile,
              const std::string& fragShaderFile,
              const std::string& albedoFile,
              const std::string& normalFile,
              const std::string& aoFile,
              const std::string& shadingFile,
              const std::string& emissionFile,
              const std::string& hdrFile,
              const std::string& modelFile);

     const VulkanShader& getVertexShader(){return vertShader;}
     const VulkanShader& getFragmentShader() {return fragShader;}
    inline const VulkanTexture &getEmissionTexture() const { return emissionTexture; }
    inline const VulkanTexture &getAlbedoTexture() const { return albedoTexture; }
    inline const VulkanTexture& getHDRTexture()const{return  hdrTexture;}
     inline const VulkanTexture &getNormalTexture() const { return normalTexture; }
     inline const VulkanTexture &getAOTexture() const { return aoTexture; }
     inline const VulkanTexture &getShadingTexture() const { return shadingTexture; }

    const VulkanMesh& getMesh() {return mesh;}


     void shutdown() ;

};

#endif //GAMEENGINE_RENDER_DATA_H
