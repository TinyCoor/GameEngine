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

class VulkanRenderScene{
private:
    VulkanRenderContext context;
    VulkanMesh mesh;
    VulkanTexture texture;
    VkShaderModule vertShader=VK_NULL_HANDLE;
    VkShaderModule fragShader=VK_NULL_HANDLE;

    std::vector<VkBuffer> uniformBuffers{};
    std::vector<VkDeviceMemory> uniformBuffersMemory{};

public:
   explicit VulkanRenderScene(VulkanRenderContext& ctx)
   :context(ctx), mesh(ctx), texture(ctx){
   }

    void init(const std::string& vertShaderFile,
              const std::string& fragShaderFile,
              const std::string& textureFile,
              const std::string& modelFile);

     inline VkShaderModule getVertexShader() const {return vertShader;}
     inline VkShaderModule getFragmentShader() const{return fragShader;}
     inline VulkanTexture getTexture() const {return texture;}
     inline const VulkanMesh getMesh() const {return mesh;}
     void shutdown();

private:
    VkShaderModule createShader(const std::string &path) const;


};

#endif //GAMEENGINE_RENDER_DATA_H
