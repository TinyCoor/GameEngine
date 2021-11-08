#ifndef _SHADER_STATE_H_
#define _SHADER_STATE_H_

layout(set =0,binding = 0) uniform RenderState {
	mat4 world;
	mat4 view;
	mat4 proj;
	vec3 cameraPosWS;
	float lerpUserValues;
	float userMetalness;
	float userRoughness;
} ubo;

#endif