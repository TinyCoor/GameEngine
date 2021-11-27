#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform UniformBuffer{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cameraPosWS;
} ubo;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(ubo.model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(ubo.model))) * aNormal;
    gl_Position =ubo.projection * ubo.view * vec4(FragPos, 1.0);
}