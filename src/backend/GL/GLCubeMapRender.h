//
// Created by 12132 on 2021/11/13.
//

#ifndef GAMEENGINE_GLCUBEMAPRENDER_H
#define GAMEENGINE_GLCUBEMAPRENDER_H
#include "GLShader.h"
#include "GLProgram.h"
#include "Buffer/GLUniformBuffer.h"
#include "GLMesh.h"

/**
 * 将HDR图像装换成 GL_TEXTURE_CUBE_MAP,
 * 用 Shader转换成 six image
 */
class GLCubeMapRender {
public:
     GLCubeMapRender(){}
    ~GLCubeMapRender(){}

    void render(GLTexture<GL_TEXTURE_CUBE_MAP>& envCubeMap);

private:
    GLProgram program;
};


#endif //GAMEENGINE_GLCUBEMAPRENDER_H
