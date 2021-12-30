#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

#include "../common/brdf.inc"
#define GBUFFER_SET 1
#include "gbuffer.inc"

#define SKY_LIGHT_SET 2
#include"../light/sky_light.inc"
#include"../common/RenderState.inc"

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outDiffuse;
layout(location = 1) out vec4 outSpecular;


void main()
{
    vec3 normalVS = texture(gbufferNormalSampler,fragTexCoord).xyz;
    normalVS.z = sqrt(1.f - dot( normalVS.xy,normalVS.xy));
    normalVS =normalize(normalVS);

    float linear_depth = texture(gbufferDepthSampler,fragTexCoord).r;
    float normalized_depth = (linear_depth -ubo.cameraParams.x)- (ubo.cameraParams.y - ubo.cameraParams.x);

    vec3 viewVS = normalize(vec3(ubo.invProj * vec4(fragTexCoord.x,fragTexCoord.y,normalized_depth,1.0)));

    vec3 shading  = texture(gbufferShadingSampler,fragTexCoord).rgb;

    SurfaceMaterial material;
    material.albedo = texture(gbufferBaseColorSampler,fragTexCoord).rgb;
    material.roughness = shading.r;
    material.metalness = shading.g;
    material.ao = 1.0;
    material.f0 = lerp(vec3(0.004),material.albedo,material.metalness);

    outDiffuse =vec4(SkyLight_Diffuse(normalVS,viewVS,material),1.0);
    outSpecular =vec4(SkyLight_Specular(normalVS,viewVS,material),1.0);
}
