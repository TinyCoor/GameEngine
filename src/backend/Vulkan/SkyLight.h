
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
namespace render::backend::vulkan{


class Light {
public:
    Light(Driver* driver, const VulkanShader* vert, const VulkanShader* frag);
    virtual ~Light();
    inline  const VulkanMesh* getMesh() const {return mesh;};
    inline  const VulkanShader* getVertexShader() const {return vertex; } ;
    inline  const VulkanShader* getFragShader() const {return fragment; };
    inline  const render::backend::BindSet* getBindSet() const {return bind_set;} ;

protected:
    Driver* driver{nullptr};
    const  VulkanShader* vertex{nullptr};
    const  VulkanShader* fragment{nullptr};
    VulkanMesh* mesh{nullptr};
    render::backend::BindSet* bind_set{nullptr};
};

class SkyLight : Light {
public:
    SkyLight(Driver* driver,const VulkanShader* vert,const  VulkanShader* frag);
    virtual ~SkyLight();

    void setBakedBRDFTexture(const Texture *brdf_texture);
    void setEnvironmentCubeMap(const Texture *env_texture);
    void setIrradianceCubeMap(const Texture *env_texture);

};

}
#endif //GAMEENGINE_SRC_BACKEND_VULKAN_SKYLIGHT_H
