//
// Created by 12132 on 2021/12/14.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_AUXILIARY_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_AUXILIARY_H
#include <volk.h>
#include "../API.h"
namespace render::backend::vulkan{

inline VkCullModeFlags toCullMode(CullMode cull_mode){
    static VkCullModeFlags modes[static_cast<uint32_t>(CullMode::MAX)] ={
        VK_CULL_MODE_NONE,
        VK_CULL_MODE_FRONT_BIT,
        VK_CULL_MODE_BACK_BIT,
        VK_CULL_MODE_FRONT_AND_BACK,
    };
    return modes[static_cast<uint32_t>(cull_mode)];
}

inline VkCompareOp toDepthCompareFunc(DepthCompareFunc depth_compare_func){
    static  VkCompareOp depth_funcs[static_cast<uint32_t>( DepthCompareFunc::MAX)] ={
        VK_COMPARE_OP_NEVER,
        VK_COMPARE_OP_LESS,
        VK_COMPARE_OP_EQUAL,
        VK_COMPARE_OP_LESS_OR_EQUAL,
        VK_COMPARE_OP_GREATER,
        VK_COMPARE_OP_NOT_EQUAL,
        VK_COMPARE_OP_GREATER_OR_EQUAL,
        VK_COMPARE_OP_ALWAYS
    };
    return depth_funcs[static_cast<uint32_t>(depth_compare_func)];
}

inline VkBlendFactor toBlendFactor(BlendFactor blend_factor){
    static VkBlendFactor factors[static_cast<uint32_t>(BlendFactor::MAX)] ={
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_SRC_COLOR,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
        VK_BLEND_FACTOR_DST_COLOR,
        VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
        VK_BLEND_FACTOR_SRC_ALPHA ,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_FACTOR_DST_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    };
    return factors[static_cast<uint32_t>(blend_factor)];
}

}

#endif //GAMEENGINE_SRC_BACKEND_VULKAN_AUXILIARY_H
