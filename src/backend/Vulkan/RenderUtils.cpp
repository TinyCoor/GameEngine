//
// Created by 12132 on 2021/12/26.
//

#include "RenderUtils.h"
#include "VulkanTexture2DRender.h"
#include "VulkanCubemapRender.h"
namespace render::backend::vulkan {

VulkanTexture *RenderUtils::createTexture2D(Driver *driver,
                                            Format format,
                                            uint32_t width,
                                            uint32_t height,
                                            uint32_t mips,
                                            const VulkanShader *vertex_shader,
                                            const VulkanShader *frag_shader)
{
    auto res = new VulkanTexture(driver);
    res->create2D(format, width, height, mips);

    VulkanTexture2DRender renderer(driver);
    renderer.init(res);
    renderer.render(vertex_shader, frag_shader);
    return res;
}

VulkanTexture *RenderUtils::createTextureCube(Driver *driver,
                                              Format format,
                                              uint32_t size,
                                              uint32_t mips,
                                              const VulkanShader *vertex_shader,
                                              const VulkanShader *frag_shader,
                                              const VulkanTexture *hdri)
{


    auto *res = new VulkanTexture(driver);
    res->createCube(backend::Format::R32G32B32A32_SFLOAT, size, size, mips);
    VulkanCubeMapRender renderer(driver);
    renderer.init(res, 0);
    renderer.render(vertex_shader, frag_shader, hdri);

    return res;
}

VulkanTexture *RenderUtils::hdriToCube( render::backend::Driver *driver,
                                        Format format,
                                        uint32_t size,
                                        const VulkanTexture* hdri,
                                        const VulkanShader* vertex_shader,
                                        const VulkanShader* hdri_frag_shader,
                                        const VulkanShader* prefilter_frag_shader)
{
    uint32_t mips = static_cast<int>(std::floor(std::log2(size)) + 1);
    auto *res = new VulkanTexture(driver);
    res->createCube(format, size, size, mips);
    VulkanCubeMapRender renderer(driver);
    renderer.init(res, 0);
    renderer.render(vertex_shader, hdri_frag_shader, hdri);

    for (uint32_t mip = 1; mip < mips ; ++mip) {
        float roughness = static_cast<float>(mip) /(float )mips;
        VulkanCubeMapRender mip_render(driver);
        mip_render.init(res,mip);
        mip_render.render(vertex_shader,
                          prefilter_frag_shader,
                          res,
                          mip - 1,
                          sizeof(float),
                          &roughness);
    }

    return res;
}
}