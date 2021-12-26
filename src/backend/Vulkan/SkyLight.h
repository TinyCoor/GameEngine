
//
// Created by 12132 on 2021/12/26.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_SKYLIGHT_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_SKYLIGHT_H
#include "driver.h"
#include "VulkanTexture.h"
#include "VulkanCubemapRender.h"
#include "VulkanTexture2DRender.h"
#include <vector>
namespace render::backend::vulkan {

class SkyLight {
public:
    SkyLight(render::backend::Driver* driver);
    virtual ~SkyLight();

    void setBRDF(const render::backend::Texture *bedf_texture);
    void setEnvironment(const render::backend::Texture *env_texture);
    inline render::backend::BindSet* getBindSet() const {return light_bind_set;}

private:
    Driver* driver{nullptr};
    render::backend::BindSet* light_bind_set{nullptr};

    VulkanTexture* baked_brdf;
    VulkanTexture* environment_cubemap;
    VulkanTexture* diffuse_irradiance_cubemap;
};

}
#endif //GAMEENGINE_SRC_BACKEND_VULKAN_SKYLIGHT_H
