//
// Created by 12132 on 2021/11/21.
//

#include "Vertex.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <tiny_obj_loader.h>
#include <assimp/cimport.h>


namespace core{

    bool loadMesh(const char *file, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) {
        Assimp::Importer importer;
        unsigned int flags = aiProcess_GenSmoothNormals |
                             aiProcess_CalcTangentSpace |
                             aiProcess_Triangulate |
                             aiProcess_JoinIdenticalVertices |
                             aiProcess_SortByPType;
        const aiScene* scene = importer.ReadFile(file,flags);
        if (!scene) {
            std::cout << "Load Model failed:"<<file << "Error: "<<importer.GetErrorString();
            return false;
        }
        if (!scene->HasMeshes()) {
            std::cerr<< "No mesh In the file\n";
            return false;
        }

        aiMesh *mesh = scene->mMeshes[0];
        assert(mesh != nullptr);
        // Fill CPU data
        vertices.resize(mesh->mNumVertices);
        indices.resize(mesh->mNumFaces * 3);

        aiVector3D *meshVertices = mesh->mVertices;
        for (unsigned int i = 0; i < mesh->mNumVertices; i++){
            vertices[i].position = glm::vec3(meshVertices[i].x, meshVertices[i].y, meshVertices[i].z);
        }

        //TODO extract method GetInfoFormMesh
        aiVector3D *meshTangents = mesh->mTangents;
        if (meshTangents) {
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                vertices[i].tangent = glm::vec3(meshTangents[i].x, meshTangents[i].y, meshTangents[i].z);
            }
        } else {
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                vertices[i].tangent = glm::vec3(0.0f, 0.0f, 0.0f);
            }
        }

        aiVector3D *meshBinormals = mesh->mBitangents;
        if (meshBinormals) {
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                vertices[i].binormal = glm::vec3(meshBinormals[i].x, meshBinormals[i].y, meshBinormals[i].z);
            }
        } else {
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                vertices[i].binormal = glm::vec3(0.0f, 0.0f, 0.0f);
            }
        }

        aiVector3D *meshNormals = mesh->mNormals;
        if (meshNormals){
            for (unsigned int i = 0; i < mesh->mNumVertices; i++){
                vertices[i].normal = glm::vec3(meshNormals[i].x, meshNormals[i].y, meshNormals[i].z);
            }
        } else{
            for (unsigned int i = 0; i < mesh->mNumVertices; i++){
                vertices[i].normal = glm::vec3(0.0f, 0.0f, 0.0f);
            }
        }

        aiVector3D *meshUVs = mesh->mTextureCoords[0];
        if (meshUVs){
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                vertices[i].uv = glm::vec2(meshUVs[i].x, 1.0f - meshUVs[i].y);
            }
        }else{
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                vertices[i].uv = glm::vec2(0.0f, 0.0f);
            }
        }


        aiColor4D *meshColors = mesh->mColors[0];
        if (meshColors){
            for (unsigned int i = 0; i < mesh->mNumVertices; i++){
                vertices[i].color = glm::vec3(meshColors[i].r, meshColors[i].g, meshColors[i].b);
            }

        } else {
            for (unsigned int i = 0; i < mesh->mNumVertices; i++){
                vertices[i].color = glm::vec3(1.0f, 1.0f, 1.0f);
            }

        }

        aiFace *meshFaces = mesh->mFaces;
        unsigned int index = 0;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            for (unsigned int faceIndex = 0; faceIndex < meshFaces[i].mNumIndices; faceIndex++){
                indices[index++] = meshFaces[i].mIndices[faceIndex];
            }

        }
        //
        return true;
    }

    bool tinyobjLoadMesh(const char *file, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file)) {
            throw std::runtime_error(warn + err);
        }
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
                vertex.color = {1.0f, 1.0f, 1.0f};
                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }
        }
        return true;
    }
}