#version 430 core

layout(location =0) in vec2 av2_pos;
layout(location =0) out vec2 vv2_pos;
void main() {
    vv2_pos = av2_pos;
    gl_Position = vec4(av2_pos, 0., 1.);
}