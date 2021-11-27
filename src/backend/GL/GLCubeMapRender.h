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
    GLCubeMapRender(GLShader& vertShader,GLShader fragShader);

    void init(GLShader& vert,GLShader& frag);

    void render();

private:
    GLProgram program;
    GLUniformBuffer uniform;
    GLMesh renderMesh;
    std::vector<GLTexture<GL_TEXTURE_2D>> textures{};
};


#endif //GAMEENGINE_GLCUBEMAPRENDER_H
