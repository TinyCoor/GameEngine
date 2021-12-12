#version 450


uniform mat4 MVPMatrix;
uniform mat4 MVMatrix; //透视之前的变换矩阵
uniform mat3 NormalMatrix;


layout(location =0) in vec4 VertexColor;
layout(location =1) in vec3 VertexNoraml;
layout(location =2) in vec4 VertexPosition;

out vec4 Color;
out vec3 Normal;
out vec4 Position;


void main()
{
    Color = VertexColor;
    Normal= noramalize(NormalMatrix *VertexNoraml);
    Position = MVMatrix * VertexPosition;       //透视前的空间
    gl_Position = MVPMatrix *VertexPosition;    //透视后的空间
}