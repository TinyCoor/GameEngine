//
// Created by 12132 on 2021/11/20.
//

#include "GLRender.h"

void GLRender::init() {
    GLShader vert(ShaderKind::vertex);
    vert.compileFromFile("../../assets/shaders/common.vert");

    GLShader frag(ShaderKind::fragment);
    frag.compileFromFile("../../assets/shaders/hdriToCube.frag");

    cubeMapRender.init(vert,frag);

}

void GLRender::render() {
    cubeMapRender.render();
}
