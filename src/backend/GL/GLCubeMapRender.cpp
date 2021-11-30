//
// Created by 12132 on 2021/11/13.
//

#include "GLCubeMapRender.h"
#include "Buffer/GLFrameBuffer.h"
#include "../core/file.h"
#include <glm/gtc/matrix_transform.hpp>

void GLCubeMapRender::render(GLTexture<GL_TEXTURE_CUBE_MAP>& envCubeMap) {

    GLShader vert(ShaderKind::vertex);
    vert.compileFromFile("../../assets/shaders/GL/common.vert");
    GLShader frag(ShaderKind::fragment);
    frag.compileFromFile("../../assets/shaders/GL/hdriToCubemap.frag");
    program.link(vert,frag);
    core::ImageInfo imageInfo;
    core::loadHDR("../../assets/textures/environment/shanghai_bund_4k.hdr",imageInfo);

    float vertices[] = {
        // back face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
        // front face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        // right face
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
        1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
        1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
        // bottom face
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
        1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
    };
    Vao cubeVao;
    GLBuffer<GL_ARRAY_BUFFER> cubeVbo;
    cubeVao.Bind();
    cubeVbo.Bind();
    cubeVbo.CopyToGPU(vertices, sizeof(vertices),GL_MAP_WRITE_BIT);

    std::vector<GLVertexAttribute> attributes{
        {0,3,8* sizeof(float ),0},
        {1,3,8* sizeof(float ),3* sizeof(float )},
        {2,2,8* sizeof(float ),6* sizeof(float )},
    };

    cubeVao.Bind();
    for (int i = 0; i < attributes.size() ; ++i) {
      glEnableVertexAttribArray(attributes[i].binding);
      glVertexAttribPointer(attributes[i].binding,attributes[i].size,GL_FLOAT,GL_FALSE,attributes[i].stride,(void*)attributes[i].offset);
    }

    GLFrameBuffer captureFBO(512,512,GL_RGB);
    GLRenderBuffer captureRBO(512,512);

    captureFBO.Bind();
    captureRBO.Bind();
    captureRBO.AllocateMemory(GL_DEPTH_COMPONENT24);
    captureFBO.AttachRenderBuffer(captureRBO);
    captureFBO.Check();

    GLTexture<GL_TEXTURE_2D> hdrTexture(imageInfo.width,imageInfo.height,0);
    hdrTexture.Bind(0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, imageInfo.width,imageInfo.height, 0, GL_RGB, GL_FLOAT, imageInfo.data); // note how we specify the texture's data value to be float
    hdrTexture.SetTextureParam( GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    hdrTexture.SetTextureParam( GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    hdrTexture.SetTextureParam( GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    hdrTexture.SetTextureParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    envCubeMap.Bind(0);
    //allocate gpu memory
    for (int i = 0; i < 6 ; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,0,GL_RGB16F,
                     512,512,0,GL_RGB,GL_FLOAT, nullptr);
    }

    envCubeMap.SetTextureParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    envCubeMap.SetTextureParam( GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    envCubeMap.SetTextureParam(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    envCubeMap.SetTextureParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    envCubeMap.SetTextureParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    envCubeMap.UnBind();

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    program.use();
    program.SetUniformInt("equirectangularMap", 0);
    program.SetUniformMatrix4fv("projection",1,GL_FALSE, captureProjection);

    glViewport(0,0,512,512);
    captureFBO.Bind();
    hdrTexture.Bind(0);
    for (unsigned int i = 0; i < 6; ++i)
    {
      program.SetUniformMatrix4fv("view",1,GL_FALSE, captureViews[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeMap.GetHandle(), 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      cubeVao.Bind();
      glDrawArrays(GL_TRIANGLES,0,36);
      cubeVao.UnBind();
    }
    captureFBO.UnBind();
    glViewport(0,0,1920,1080);
}
