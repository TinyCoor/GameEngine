//
// Created by 12132 on 2021/12/26.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_RENDERUTILS_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_RENDERUTILS_H
#include "driver.h"
#include "VulkanTexture.h"
#include "VulkanShader.h"
///todo move out vulkan dir
namespace render::backend::vulkan {

class RenderUtils {
public:
    static VulkanTexture* createTexture2D(
        Driver* driver,
        Format format,
        uint32_t width,
        uint32_t height,
        uint32_t mips,
        const VulkanShader* vertex_shader,
        const VulkanShader* frag_shader);

    static VulkanTexture* createTextureCube(
        render::backend::Driver* driver,
        Format format,
        uint32_t size,
        uint32_t mips,
        const VulkanShader* vertex_shader,
        const VulkanShader* frag_shader,
        const VulkanTexture* input
        );

    static VulkanTexture* hdriToCube(
        render::backend::Driver* driver,
        Format format,
        uint32_t size,

        const VulkanTexture* hdri,
        const VulkanShader* vertex_shader,
        const VulkanShader* hdri_frag_shader,
        const VulkanShader* prefilter_frag_shader
    );



};
}
#endif //GAMEENGINE_SRC_BACKEND_VULKAN_RENDERUTILS_H
