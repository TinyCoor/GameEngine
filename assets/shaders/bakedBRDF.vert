#version 450
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

// Input
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inTangent;
layout(location = 2) in vec3 inBinormal;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inColor;
layout(location = 5) in vec2 inTexCoord;

// Output
layout(location = 0) out vec3 position;
layout(location = 1) out vec2 uv;

void main() {
    gl_Position = vec4(inPosition.xyz, 1.0f);


    position =inPosition;
    uv = inTexCoord;
}
