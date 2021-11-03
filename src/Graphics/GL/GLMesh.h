//
// Created by y123456 on 2021/10/18.
//

#ifndef GAMEENGINE_GLMESH_H
#define GAMEENGINE_GLMESH_H
#include <string>
#include <glm/glm.hpp>
#include "VAO.h"

struct Vertex{
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 normal;
};

class GLMesh {
public:
    GLMesh();
    ~GLMesh();

    bool loadMesh(const std::string& file);

private:
   Vao vao;

};


#endif //GAMEENGINE_GLMESH_H
