//
// Created by 12132 on 2021/11/21.
//

#ifndef GAMEENGINE_VERTEX_H
#define GAMEENGINE_VERTEX_H
#include <glm/glm.hpp>
#include <vector>
#include <assimp/mesh.h>

namespace core {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 tangent;
        glm::vec3 binormal;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec2 uv;
    };

    struct Texture {
        uint8_t * texture_data;
        const char* texture_path;
        uint32_t  width;
        uint32_t  height;
        uint8_t  one_pixel_size; ///RGBA8
        uint32_t total_size;
    };

    //class Mesh {
    //public:
    //    Mesh() =default;
    //    virtual ~Mesh() =default;
    //
    //    bool import(const char* path);
    //
    //protected:
    //    std::vector<Vertex> vertices;  //顶点数据
    //    std::vector<uint32_t> indices; //顶点索引
    //    std::vector<Texture> textures;
    //};


    bool import(const char* path);

    bool import(const char *name, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

    bool import_scene(const char* path);

    bool import(const aiMesh* mesh,std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

//    bool tinyobjLoadMesh(const char *name, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

    void generateSphere();
}

#endif //GAMEENGINE_VERTEX_H
