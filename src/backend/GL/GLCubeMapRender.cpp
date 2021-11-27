//
// Created by 12132 on 2021/11/13.
//

#include "GLCubeMapRender.h"
#include "Buffer/GLFrameBuffer.h"
#include <glm/gtc/matrix_transform.hpp>

struct CubemapFaceOrientationData{
    glm::mat4 faces[6];
};

void GLCubeMapRender::init(GLShader &vert, GLShader &frag) {
    program.link(vert,frag);
    renderMesh.createQuad(2.0);

    for (int i = 0; i < 6 ; ++i) {
        GLTexture<GL_TEXTURE_2D> texture(1920,1080,0,0,GL_RGBA);
        textures.push_back(texture);
    }


    uniform.init(sizeof(CubemapFaceOrientationData));

    uniform.BindUniformBuffer(0,program,"RenderState");
    CubemapFaceOrientationData* ubo = (CubemapFaceOrientationData*)uniform.Map(GL_READ_ONLY);
    const glm::mat4 &translateZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    const glm::vec3 faceDirs[6] = {
            glm::vec3( 1.0f,  0.0f,  0.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3( 0.0f,  1.0f,  0.0f),
            glm::vec3( 0.0f, -1.0f,  0.0f),
            glm::vec3( 0.0f,  0.0f,  1.0f),
            glm::vec3( 0.0f,  0.0f, -1.0f),
    };

    const glm::vec3 faceUps[6] = {
            glm::vec3( 0.0f,  0.0f, -1.0f),
            glm::vec3( 0.0f,  0.0f,  1.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3( 0.0f, -1.0f,  0.0f),
            glm::vec3( 0.0f, -1.0f,  0.0f),
    };

    const glm::mat4 faceRotations[6] = {
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            glm::mat4(1.0f),
            glm::mat4(1.0f),
            glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    };
    for (int i = 0; i < 6; i++)
        ubo->faces[i] = faceRotations[i] * glm::lookAtRH(glm::vec3(0.0f), faceDirs[i], faceUps[i]) * translateZ;

    uniform.UnMap();

}

void GLCubeMapRender::render() {
    auto sampler = createSamler<GL_TEXTURE_2D>();

    GLFrameBuffer frameBuffer(1920,1080, sizeof(uint32_t));
    for (int i = 0; i <textures.size() ; ++i) {
        frameBuffer.BindColorTexture(textures[i].GetHandle(),i,0);
    }
    frameBuffer.Bind();
    sampler.BindSampler(0);

    renderMesh.draw(program);

    frameBuffer.UnBind();

}
