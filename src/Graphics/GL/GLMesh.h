//
// Created by y123456 on 2021/10/18.
//

#ifndef GAMEENGINE_GLMESH_H
#define GAMEENGINE_GLMESH_H
#include <string>
#include <glm/glm.hpp>
#include "VAO.h"
#include "GLBuffer.hpp"

struct Vertex{
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 normal;
};

class GLMesh {
public:
    GLMesh();
    ~GLMesh();

    bool loadFromFile(const char* file);
    

private:

   Vao vao;
   GLBuffer<GL_ARRAY_BUFFER> vbo;

};


#endif //GAMEENGINE_GLMESH_H
