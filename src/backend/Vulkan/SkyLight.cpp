//
// Created by 12132 on 2021/12/26.
//

#include "SkyLight.h"
namespace render::backend::vulkan{

SkyLight::SkyLight(render::backend::Driver* driver):driver(driver)
{
    light_bind_set = driver->createBindSet();
}

SkyLight::~SkyLight()
{

}

void SkyLight::setBRDF(const render::backend::Texture *bedf_texture)
{

}

void SkyLight::setEnvironment(const render::backend::Texture *env_texture)
{
}
}