#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

#define SKY_LIGHT_SET 2
#include "light/sky_light.inc"


layout(set =0,binding = 0) uniform RenderState {
	mat4 world;
	mat4 view;
	mat4 proj;
	vec3 cameraPos;
} ubo;

layout(set =1,binding = 5) uniform samplerCube environmentSampler;
layout(set =1,binding = 6) uniform samplerCube diffuseIrradianceSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPositionOS;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 color = texture(skylightEnvironmentSampler, normalize(fragPositionOS)).rgb;

	// TODO: move to separate pass
	// Tonemapping + gamma correction
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));

	outColor = vec4(color, 1.0f);
}
