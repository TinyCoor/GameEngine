//
// Created by 12132 on 2021/12/26.
//

#include "SkyLight.h"
#
namespace render::backend::vulkan{

Light::Light(Driver *driver, const VulkanShader* vert, const VulkanShader* frag)
        :driver(driver),vertex(vert),fragment(frag)
{
    bind_set = driver->createBindSet();
}

Light::~Light()
{
    driver->destroyBindSet(bind_set);
    bind_set = nullptr;
    mesh = nullptr;
    vertex= nullptr;
    fragment= nullptr;
}

SkyLight::SkyLight(Driver* driver, const VulkanShader* vert, const VulkanShader* frag)
    : Light(driver,vert,frag)
{
    mesh = new VulkanMesh(driver);
    mesh->createQuad(2.f);
}

SkyLight::~SkyLight()
{

}
void SkyLight::setBakedBRDFTexture(const VulkanTexture *brdf_texture)
{
    driver->bindTexture(bind_set,0,brdf_texture->getTexture());
}

void SkyLight::setEnvironmentCubeMap(const VulkanTexture *env_texture)
{
    driver->bindTexture(bind_set,1,env_texture->getTexture());
}

void SkyLight::setIrradianceCubeMap(const VulkanTexture *ir_texture)
{
    driver->bindTexture(bind_set,2,ir_texture->getTexture());
}

}