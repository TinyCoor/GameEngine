//
// Created by 12132 on 2021/11/21.
//

#ifndef GAMEENGINE_VERTEX_H
#define GAMEENGINE_VERTEX_H
#include <glm/glm.hpp>
#include <vector>

namespace core {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 tangent;
        glm::vec3 binormal;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec2 uv;
    };

    bool loadMesh(const char *name, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
    bool tinyobjLoadMesh(const char *name, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

    void generateSphere();
}

#endif //GAMEENGINE_VERTEX_H
