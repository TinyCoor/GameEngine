//
// Created by 12132 on 2021/12/14.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_AUXILIARY_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_AUXILIARY_H
#include "driver.h"
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <volk.h>

namespace render::backend::vulkan {

template<class T>
static void hash_combine(uint64_t &s, const T &v)
{
    std::hash<T> h;
    s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}
static VkShaderStageFlagBits toShaderStage(ShaderType type)
{
    static VkShaderStageFlagBits supported_stages[] =
        {
            // Graphics pipeline
            VK_SHADER_STAGE_VERTEX_BIT,
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            VK_SHADER_STAGE_GEOMETRY_BIT,
            VK_SHADER_STAGE_FRAGMENT_BIT,

            // Compute pipeline
            VK_SHADER_STAGE_COMPUTE_BIT,

            // Raytracing pipeline
            // TODO: subject of change, because Khronos announced
            // vendor independent raytracing support in Vulkan
            VK_SHADER_STAGE_RAYGEN_BIT_NV,
            VK_SHADER_STAGE_INTERSECTION_BIT_NV,
            VK_SHADER_STAGE_ANY_HIT_BIT_NV,
            VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV,
            VK_SHADER_STAGE_MISS_BIT_NV,
            VK_SHADER_STAGE_CALLABLE_BIT_NV,

        };

    return supported_stages[static_cast<int>(type)];
};


void createTextureData(const Device *context, Texture *texture,
                       Format format, const void *data,
                       int num_data_mipmaps, int num_data_layer);

void selectOptimalSwapChainSettings(Device *context, SwapChain *swapchain);

bool createSwapchainObjects(Device *context,
                            SwapChain *swapchain,
                            uint32_t width,
                            uint32_t height);

void destroySwapchainObjects(Device *context, SwapChain *swapchain);

void updateBindSetLayout(const Device *device, BindSet *bind_set, VkDescriptorSetLayout new_layout);

}// namespace render::backend::vulkan

#endif//GAMEENGINE_SRC_BACKEND_VULKAN_AUXILIARY_H
