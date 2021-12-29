#ifndef SKYLIGHT_H_
#define SKYLIGHT_H_

#include"../common/brdf.inc"

layout(set=SKY_LIGHT_SET,binding=0) uniform sampler2D skylightBrdfSampler;
layout(set=SKY_LIGHT_SET,binding=1) uniform samplerCube skylightEnvironmentSampler;
layout(set=SKY_LIGHT_SET,binding=2) uniform samplerCube skyLightIrradianceSampler;


vec3 SkyLight_Diffuse(vec3 normal,vec3 view,SurfaceMaterial material)
{
	float dotNV = max(0.0f, dot(normal, view));
    vec3 F = F_Shlick(dotNV, material.f0, material.roughness);
    vec3 irradiance = texture(skyLightIrradianceSampler, normal).rgb * material.ao;
    vec3 res = irradiance * material.albedo;
    res *= lerp(vec3(1.0f) - F, vec3(0.0f), material.metalness);
    return res;
}

vec3 Specular_Specular(vec3 normal,vec3 view,SurfaceMaterial material)
{
    float dotNV = max(0.0f, dot(normal, view));
    float mip = material.roughness * 8.0f;
    vec3 Li = textureLod(skylightEnvironmentSampler, -reflect(view, normal), mip).rgb;
    vec2 brdf = texture(skylightBrdfSampler, vec2(material.roughness, dotNV)).xy;

    return Li * (material.f0 * brdf.x + brdf.y);
}

#endif