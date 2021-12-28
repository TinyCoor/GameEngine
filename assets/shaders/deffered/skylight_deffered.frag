#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

#include "common/brdf.inc"
#define GBUFFER_SET 0
#include "deffered/gbuffer.inc"

#define SKY_LIGHT_SET 1
#include"light/sky_light.inc"


layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outDiffuse;
layout(location = 1) out vec4 outSpecular;


void main()
{
    vec3 normalVS = texture(gbufferNormalSampler,fragTexCoord).xyz;
    normalVS.z = sqrt(1.f - dot( normalVS.xy,normalVS.xy));
    normalVS =normalize(normalVS);

    float depth = texture(gbufferDepthSampler,fragTexCoord).r;
    vec3 viewVS = vec3(0.f,0.f,1.f);

    vec3 shading  = texture(gbufferShadingSampler,fragTexCoord).rg;


    SurfaceMaterial material;
    material.albedo = texture(gbufferBaseColorSampler,fragTexCoord).rgb;
    material.roughness = shading.r;
    material.metalness = shading.g;
    material.ao = 1.0;
    material.f0 = lerp(vec3(0.004),material.albedo,material.metalness);

    outDiffuse =SkyLight_Diffuse(normalVS,viewVS,material);
    outSpecular =Specular_Specular(normalVS,viewVS,material);



}
