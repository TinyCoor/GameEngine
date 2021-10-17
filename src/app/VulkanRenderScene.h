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

#include "Macro.h"

class IModelLoader{
public:
    virtual VulkanMesh loadModel(const std::string& path) =0;
};

class RenderScene{
public:
    virtual void init(const std::string& vertShaderFile,
                      const std::string& fragShaderFile,
                      const std::string& textureFile,
                      const std::string& modelFile)=0;
    virtual VkShaderModule getVertexShader()  =0;
    virtual VkShaderModule getFragmentShader()  =0;
    virtual VulkanTexture getTexture() =0;
    virtual VulkanMesh getMesh() =0;
    virtual  void shutdown()=0;
};

class VulkanRenderScene : public RenderScene{
private:
    VulkanRenderContext context;
    VulkanMesh mesh;
    VulkanTexture texture;

    VkShaderModule vertShader=VK_NULL_HANDLE;
    VkShaderModule fragShader=VK_NULL_HANDLE;


public:
   explicit VulkanRenderScene(VulkanRenderContext& ctx)
   :context(ctx), mesh(ctx), texture(ctx){
   }

    void init(const std::string& vertShaderFile,
              const std::string& fragShaderFile,
              const std::string& textureFile,
              const std::string& modelFile);

     VkShaderModule getVertexShader()override  {return vertShader;}
     VkShaderModule getFragmentShader()override {return fragShader;}
     VulkanTexture getTexture()override {return texture;}
     VulkanMesh getMesh() override {return mesh;}
     void shutdown();

private:
    VkShaderModule createShader(const std::string &path) const;


};

#endif //GAMEENGINE_RENDER_DATA_H
