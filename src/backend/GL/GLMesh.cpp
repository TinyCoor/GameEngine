//
// Created by y123456 on 2021/10/18.
//

#include "GLMesh.h"

#include <assimp/scene.h>

// 针对单个mesh
bool GLMesh::loadFromFile(const char* file)
{
    clear();
    core::loadMesh(file,vertices,indices);
    std::cout<< vertices.size() << " indices: "<< indices.size();
    uploadToGPU();
    return true;
}


void GLMesh::uploadToGPU(){

    const size_t SizeIndices = sizeof(uint32_t) * indices.size();
    const size_t SizeVertices = sizeof(core::Vertex) * vertices.size();

    vao.Bind();
    vbo.Bind();
    vbo.CopyToGPU(vertices.data(),SizeVertices,GL_MAP_READ_BIT);
    ebo.Bind();
    ebo.CopyToGPU(indices.data(),SizeIndices,GL_MAP_READ_BIT);
    glVertexArrayElementBuffer(vao.GetHandle(),ebo.GetHandle());

    std::vector<GLVertexAttribute> attributes{
        { 0,3,sizeof(core::Vertex) ,offsetof(core::Vertex, position) },
        { 1,3,sizeof(core::Vertex) ,offsetof(core::Vertex, tangent) },
        { 2,3,sizeof(core::Vertex) ,offsetof(core::Vertex, binormal) },
        { 3,3,sizeof(core::Vertex) ,offsetof(core::Vertex, normal) },
        { 4,3,sizeof(core::Vertex) ,offsetof(core::Vertex, color) },
        { 5,2,sizeof(core::Vertex) ,offsetof(core::Vertex, uv) },
    };

    for (int i = 0; i < attributes.size(); ++i) {
        glVertexAttribPointer(attributes[i].binding,attributes[i].size,
                              GL_FLOAT,GL_FALSE, attributes[i].stride,(void*)attributes[i].offset);
        glEnableVertexAttribArray(attributes[i].binding);
    }
}

void GLMesh::clear() {
    vertices.clear();
    indices.clear();
}

void GLMesh::draw(GLProgram &program) {
    vao.Bind();
    glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,0);
    vao.UnBind();
}

void GLMesh::draw() {
  vao.Bind();
  glDrawElements(GL_QUADS,indices.size(),GL_UNSIGNED_INT,0);
  vao.UnBind();
}



void GLMesh::createQuad(float size) {
    float halfSize = size * 0.5f;
    vertices.resize(4);
    indices.resize(6);

    vertices[0].position = glm::vec3(-halfSize, -halfSize, 0.0f);
    vertices[1].position = glm::vec3( halfSize, -halfSize, 0.0f);
    vertices[2].position = glm::vec3( halfSize,  halfSize, 0.0f);
    vertices[3].position = glm::vec3(-halfSize,  halfSize, 0.0f);

    vertices[0].uv = glm::vec2(0.0f, 0.0f);
    vertices[1].uv = glm::vec2(1.0f, 0.0f);
    vertices[2].uv = glm::vec2(1.0f, 1.0f);
    vertices[3].uv = glm::vec2(0.0f, 1.0f);

    indices = {
            1, 0, 2, 3, 2, 0,
    };

    uploadToGPU();
}
void GLMesh::createSkyBox(float size) {
  size *= 0.5;
  vertices.resize(8);
  indices.resize(36);
  vertices[0].position = glm::vec3(-size, -size, -size);
  vertices[1].position = glm::vec3(size, -size, -size);
  vertices[2].position = glm::vec3(size, size, -size);
  vertices[3].position = glm::vec3(-size, size, -size);

  vertices[4].position = glm::vec3(-size, -size, size);
  vertices[5].position = glm::vec3(size, -size, size);
  vertices[6].position = glm::vec3(size, size, size);
  vertices[7].position = glm::vec3(-size, size, size);

  indices = {
      0, 1, 2, 2, 3, 0,
      1, 5, 6, 6, 2, 1,
      3, 2, 6, 6, 7, 3,
      5, 4, 6, 4, 7, 6,
      1, 0, 4, 4, 5, 1,
      4, 0, 3, 3, 7, 4,
  };

  uploadToGPU();
}
