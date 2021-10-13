//
// Created by y123456 on 2021/10/12.
//

#ifndef GAMEENGINE_RENDER_DATA_H
#define GAMEENGINE_RENDER_DATA_H
#include "vulkan_utils.h"
#include "vertex.h"
#include "utils.hpp"
#include <vulkan.h>
#include <vector>
#include <string>

class RenderData{
private:
    RenderContext context;
    VkShaderModule vertShader=VK_NULL_HANDLE;
    VkShaderModule fragShader=VK_NULL_HANDLE;

    VkBuffer vertexBuffer =VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory =VK_NULL_HANDLE;

    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory= VK_NULL_HANDLE;



public:
   explicit RenderData(RenderContext& ctx): context(ctx){
   }

    void init(const std::string& vertShaderFile,
              const std::string& fragShaderFile);

    inline VkShaderModule getVertexShader() const {return vertShader;}
    inline VkShaderModule getFragmentShader() const{return fragShader;}
    inline VkBuffer getVertexBuffer() const{return vertexBuffer;}
    inline VkBuffer getIndexBuffer() const{return indexBuffer;}


    void shutdown();

private:
    VkShaderModule createShader(const std::string &path) const ;
    void createIndexBuffer();
    void createVertexBuffer();

};

#endif //GAMEENGINE_RENDER_DATA_H
