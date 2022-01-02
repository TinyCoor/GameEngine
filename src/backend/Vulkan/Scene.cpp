//
// Created by 12132 on 2021/12/25.
//

#include "Scene.h"
#include "VulkanTexture.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <sstream>
#include <iostream>

namespace render::backend::vulkan {
static glm::mat4 toGlmMat(aiMatrix4x4 matrix)
{
    glm::mat4 res;
    res[0].x = matrix.a1,res[1].x = matrix.a2,res[2].x = matrix.a3,res[3].x = matrix.a4;
    res[0].y = matrix.b1,res[1].y = matrix.b2,res[2].y = matrix.b3,res[3].y = matrix.b4;
    res[0].z = matrix.c1,res[1].z = matrix.c2,res[2].z = matrix.c3,res[3].z = matrix.c4;
    res[0].w = matrix.d1,res[1].w = matrix.d2,res[2].w = matrix.d3,res[3].w = matrix.d4;
    return res;
}

static render::backend::Texture* default_albedo= nullptr;
static render::backend::Texture* default_normal= nullptr;
static render::backend::Texture* default_metalness= nullptr;
static render::backend::Texture* default_roughness = nullptr;

static void generateDefaultTexture(Driver* driver) {

    auto gen_texture= [=](uint8_t r,uint8_t g,uint8_t b) ->render::backend::Texture*{
        uint8_t pixel[16]= {
            r,g,b,255,
            r,g,b,255,
            r,g,b,255,
            r,g,b,255,
        };

        return  driver->createTexture2D(2,2,1,
                                        render::backend::Format::R8G8B8A8_UNORM,
                                        Multisample::COUNT_1,pixel
        );

    };
    if (default_albedo == nullptr)
        default_albedo= gen_texture(127,127,127);

    if(default_normal == nullptr)
        default_normal= gen_texture(127,127,255);
    if(default_metalness == nullptr)
        default_metalness = gen_texture(0,0,0);
    if (default_roughness == nullptr)
        default_roughness = gen_texture(255,255,255);

}

Scene::Scene(render::backend::Driver *driver) :driver(driver)
{
}

Scene::~Scene()
{
    clear();
}

bool Scene::import(const char *path)
{
    generateDefaultTexture(driver);
    Assimp::Importer importer;
    unsigned int flags = aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType;
    const aiScene *scene = importer.ReadFile(path, flags);
    if (!scene) {
        return false;
    }
    if (!scene->HasMeshes()) {
        return false;
    }

    clear();

    std::string dir = path;
    auto pos = dir.find_last_of('/');
    if(pos != std::string::npos) {
        dir = dir.substr(0,pos);
    }

    ///  import mesh
    meshes.resize(scene->mNumMeshes);
    for (uint32_t i = 0; i < scene->mNumMeshes ; ++i) {
        auto mesh =  new VulkanMesh(driver);
        mesh->import(scene->mMeshes[i]);
        meshes[i] =mesh;
    }

    ///  import texture
    textures.clear();
    for (uint32_t i = 0; i <scene->mNumTextures ; ++i) {
        auto* texture = new VulkanTexture(driver);
        std::stringstream path_builder;
        path_builder << dir << '/' << scene->mTextures[i]->mFilename.C_Str();
        texture->import(path_builder.str().c_str());
        textures[path_builder.str()] = texture;
    }

    /// import material
    auto import_material_texture =[=](aiMaterial* material, aiTextureType type)->VulkanTexture* {
        aiString path;
        material->GetTexture(type,0,&path);
        if(path.length ==0)
            return nullptr;

        std::stringstream path_builder;
        path_builder<< dir <<'/' << path.C_Str();

        const std::string& texture_path = path_builder.str();

        auto it = textures.find(path.C_Str());
        if (it == textures.end()){
            VulkanTexture* texture = new VulkanTexture(driver);
            texture->import(texture_path.c_str());
            textures[texture_path]=texture;
            return texture;
        }
        return it->second;
    };

    materials.resize(scene->mNumMaterials);
    for (uint32_t i = 0; i <scene->mNumMaterials ; ++i) {
        auto material = scene->mMaterials[i];
        RenderMaterial& render_material = materials[i];
        render_material.albedo = import_material_texture(material,aiTextureType_DIFFUSE);
        render_material.normal = import_material_texture(material,aiTextureType_NORMALS);
        render_material.roughness =import_material_texture(material,aiTextureType_SHININESS);
        render_material.metallic =import_material_texture(material,aiTextureType_AMBIENT);

        render_material.bind_set = driver->createBindSet();

        const render::backend::Texture* albedo   = render_material.albedo   ? render_material.albedo->getTexture() : default_albedo;
        const render::backend::Texture* normal   = render_material.normal   ? render_material.normal->getTexture() : default_normal;
        const render::backend::Texture* roughness= render_material.roughness? render_material.roughness->getTexture() : default_roughness;
        const render::backend::Texture* metallic = render_material.metallic ? render_material.metallic->getTexture() : default_metalness ;

        driver->bindTexture(render_material.bind_set,0,albedo);
        driver->bindTexture(render_material.bind_set,1,normal);
        driver->bindTexture(render_material.bind_set,2,roughness);
        driver->bindTexture(render_material.bind_set,3,metallic);

    }

    /// import nodes;
    aiNode* root = scene->mRootNode;
    import_nodes(scene,root, toGlmMat(root->mTransformation));
    return true;
}

void Scene::clear()
{
    for (size_t i = 0; i < meshes.size(); ++i) {
        delete meshes[i];
        meshes[i]= nullptr;
    }
    for (size_t i = 0; i < materials.size(); ++i) {
        driver->destroyBindSet(materials[i].bind_set);
        meshes[i]= {};
    }

    for (auto& texture: textures) {
        delete texture.second;
        texture.second= nullptr;
    }

    meshes.clear();
    materials.clear();
    textures.clear();
}

void Scene::import_nodes(const aiScene* scene,const aiNode *root,const glm::mat4& transform)
{
    for (int i = 0; i < root->mNumMeshes ; ++i) {
        uint32_t mesh_index=root->mMeshes[i];
        uint32_t material_index = scene->mMeshes[mesh_index]->mMaterialIndex;

        auto* mesh = meshes[mesh_index];

        RenderNode node;
        node.mesh = mesh;
        node.transform = transform;
        node.render_material_index = material_index;

        nodes.push_back(node);
    }

    for (int i = 0; i <root->mNumChildren ; ++i) {
        const aiNode* child = root->mChildren[i];
        const glm::mat4& child_transform = transform * toGlmMat(child->mTransformation);
        import_nodes(scene,child,child_transform);
    }
}

}
