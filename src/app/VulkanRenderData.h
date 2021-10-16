//
// Created by y123456 on 2021/10/12.
//

#ifndef GAMEENGINE_RENDER_DATA_H
#define GAMEENGINE_RENDER_DATA_H
#include "VulkanUtils.h"
#include "VulkanRenderContext.h"
#include "VulkanMesh.h"
#include <vulkan.h>
#include <vector>
#include <string>

#include "Macro.h"

class IModelLoader{
public:
    virtual VulkanMesh loadModel(const std::string& path) =0;
};

class RenderData{
private:
    RenderContext context;
    VulkanMesh mesh;

    VkShaderModule vertShader=VK_NULL_HANDLE;
    VkShaderModule fragShader=VK_NULL_HANDLE;

    std::vector<VkBuffer> uniformBuffers{};
    std::vector<VkDeviceMemory> uniformBuffersMemory{};

    VkImage textureImage{};
    VkDeviceMemory textureImageMemory{};

    VkImageView textureImageView{};
    VkSampler textureImageSampler{};

public:
   explicit RenderData(RenderContext& ctx): context(ctx), mesh(ctx){
   }

    void init(const std::string& vertShaderFile,
              const std::string& fragShaderFile,
              const std::string& textureFile,
              const std::string& modelFile);

     inline VkShaderModule getVertexShader() const {return vertShader;}
     inline VkShaderModule getFragmentShader() const{return fragShader;}
     inline VkImageView getTextureImageView() const {return textureImageView;}
     inline VkSampler  getTextureImageSmapler()const {return textureImageSampler;}
     inline const VulkanMesh getMesh() const {return mesh;}
     void shutdown();

private:
    VkShaderModule createShader(const std::string &path) const;
    void createImage(const std::string& path);

};

#endif //GAMEENGINE_RENDER_DATA_H
