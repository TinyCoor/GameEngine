#version 450


uniform mat4 MVPMatrix;
uniform mat3 NormalMatrix;

layout(location =0) in vec4 VertexColor;
layout(location =1) in vec3 VertexNoraml;
layout(location =2) in vec4 VertestPosition;

out vec4 Color;
out vec3 Normal;

void main()
{
    Color = VertexColor;
    Normal= noramalize(NormalMatrix *VertexNoraml);
    gl_Position = MVPMatrix *VertestPosition;
}