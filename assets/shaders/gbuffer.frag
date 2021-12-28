#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

#include "RenderState.inc"
#include "common/brdf.inc"



layout(set=1,binding = 0) uniform sampler2D albedoSampler;
layout(set=1,binding = 1) uniform sampler2D normalSampler;
layout(set=1,binding = 2) uniform sampler2D roughnessSampler;
layout(set=1,binding = 3) uniform sampler2D metailicSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragTangentVS;
layout(location = 3) in vec3 fragBinormalVS;
layout(location = 4) in vec3 fragNormalVS;
layout(location = 5) in vec3 fragPositionVS;

layout(location = 0) out vec4 outBaseColor;
layout(location = 1) out float outDepth;
layout(location = 2) out vec2 outNormal;
layout(location = 3) out vec2 outShading;

vec3 DirectBRDF(Surface surface, SurfaceMaterial material)
{
	vec3 diffuse;
	vec3 specular;
	GetBRDF(surface,material,diffuse,specular);

	return diffuse+specular;
}

void main()
{

	vec3 normalVS = texture(normalSampler, fragTexCoord).xyz * 2.0f - vec3(1.0f);

	mat3 tangentToVs;
	tangentToVs[0] = normalize(fragTangentVS);
	tangentToVs[1] = normalize(-fragBinormalVS);
	tangentToVs[2] = normalize(fragNormalVS);

	normalVS = normalize(tangentToVs * normalVS);
	vec3 albedo = texture(albedoSampler, fragTexCoord).rgb;
	float roughness = texture(roughnessSampler, fragTexCoord).r;
	float metalness = texture(metailicSampler, fragTexCoord).r;

	outBaseColor =vec4(albedo,1.0);
	outNormal = normalVS.xy * 0.5 + vec2(0.5f);
	outDepth =length(fragPositionVS);
	outShading =vec2(roughness, metalness);
}

