#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragTangentWS;
layout(location = 3) in vec3 fragBinormalWS;
layout(location = 4) in vec3 fragNormalWS;
layout(location = 5) in vec3 fragPositionWS;

layout(location = 0) out vec4 outColor;


void main()
{

    outColor = vec4(1.0f);
}
