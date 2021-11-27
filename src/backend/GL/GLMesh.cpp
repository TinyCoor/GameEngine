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
    std::cout<<vertices.size() << "indices: "<< indices.size();
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
//    glVertexArrayElementBuffer(vao.GetHandle(),ebo.GetHandle());

    std::vector<GLVertexAttribute> attributes{
            { 0,3,offsetof(core::Vertex, position) },
            { 1,3,offsetof(core::Vertex, tangent) },
            { 2,3,offsetof(core::Vertex, binormal) },
            { 3,3,offsetof(core::Vertex, normal) },
            { 4,3,offsetof(core::Vertex, color) },
            { 5,2,offsetof(core::Vertex, uv) },
    };

    for (int i = 0; i < attributes.size(); ++i) {
        glVertexAttribPointer(attributes[i].binding,attributes[i].size,
                              GL_FLOAT,GL_FALSE, sizeof(core::Vertex),(void*)attributes[i].offset);
        glEnableVertexAttribArray(attributes[i].binding);
    }


}


void GLMesh::clear() {
    vertices.clear();
    indices.clear();
}

void GLMesh::draw(GLProgram &program) {
    vao.Bind();
    ebo.Bind();
    glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,0);
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
