//
// Created by y123456 on 2021/10/18.
//

#ifndef GAMEENGINE_GLMESH_H
#define GAMEENGINE_GLMESH_H
#include <string>
#include "VAO.h"
#include "Buffer/GLBuffer.hpp"
#include "GLTexture.h"
#include "../core/Vertex.h"

class RenderObject{
public:
    virtual ~RenderObject()=default;
    virtual void draw(GLProgram& program) = 0;
};

//用于通知GPU 数据分布样式
struct GLVertexAttribute{
    int binding;
    int size;
    int stride;
    size_t offset;
};

class GLMesh  : public RenderObject {
public:
    GLMesh() = default;
    ~GLMesh()=default;

    bool loadFromFile(const char* file);

    void clear();

    void createQuad(float size);

    void createSkyBox(float size);

    void draw(GLProgram& program) override;
    void draw() ;

private:
    void uploadToGPU();
private:
    std::vector<core::Vertex> vertices{};
    std::vector<uint32_t> indices;
    //顶点数据
    Vao vao;
    GLBuffer<GL_ARRAY_BUFFER> vbo;
    GLBuffer<GL_ELEMENT_ARRAY_BUFFER> ebo;
    std::vector<GLTexture<GL_TEXTURE_2D>> textures;
    //maybe need some uniform buffer
};


#endif //GAMEENGINE_GLMESH_H
