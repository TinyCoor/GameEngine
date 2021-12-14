//
// Created by 12132 on 2021/12/14.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_CONTEXT_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_CONTEXT_H
#include <vector>
#include "driver.h"
namespace render::backend::vulkan {
/**
 *
 */
class context {
public:
    context();
    ~context();
    void clearBindSets();
    void clearShaders();
    void pushBindSet(const BindSet* bind_set);
    void setBindSet(uint8_t binding,const BindSet* set);

    // setter
    inline void setRenderPass(VkRenderPass render_pass) {current_pass = render_pass;}
    void setShader(ShaderType type, const Shader* shader);
    inline void setCullMode(VkCullModeFlags cull_mode) { state.cull_mode = cull_mode;}
    inline void setBlendFactor(VkBlendFactor src_factor,VkBlendFactor dst_factor) {
        state.blend_src_factor = src_factor;
        state.blend_dst_factor = dst_factor;
    }

    inline void setDepthCompareFunc(VkCompareOp compare_func) {state.depth_compare_func =compare_func;}
    inline void setDepthTest(bool enabled) {state.depth_test =enabled;}
    inline void setDepthWrite(bool enabled) {state.depth_write=enabled;}
    inline void setBlending(bool enabled) {state.blending = enabled;}


    // getters
    VkRenderPass getRenderPass() const{return current_pass;}
    inline VkShaderModule getShader(ShaderType type) { return shaders[static_cast<uint32_t>(type)];}
    inline VkCullModeFlags getCullMode() const {return state.cull_mode;}
    inline VkBlendFactor getBlendSrcFactor() const { return state.blend_src_factor;}
    inline VkBlendFactor getBlendDstFactor() const { return state.blend_dst_factor;}
    inline VkCompareOp  getDepthCompareFunc() const { return state.depth_compare_func;}
    inline bool isDepthTestEnable() const { return state.depth_test == 1; }
    inline bool isDepthWriteEnable() const { return state.depth_write == 1;}
    inline bool isBlendingEnable() const { return  state.blending == 1;}

private:
    void clear();
    struct RenderState{
        VkCullModeFlags cull_mode : 2;
        uint8_t depth_test : 1;
        uint8_t depth_write : 1;
        uint8_t blending : 1;
        VkCompareOp depth_compare_func ;
        VkBlendFactor blend_src_factor ;
        VkBlendFactor blend_dst_factor ;
    };

private:
    enum {
        MAX_SET =16
    };
    BindSet set[MAX_SET];
    uint8_t num_sets{0};

    VkShaderModule shaders[static_cast<uint32_t>(ShaderType::MAX)];

    RenderState state;
    VkRenderPass current_pass{VK_NULL_HANDLE};

};

}

#endif //GAMEENGINE_SRC_BACKEND_VULKAN_CONTEXT_H
