//
// Created by 12132 on 2021/12/25.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_SCENE_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_SCENE_H
#include "driver.h"
#include "VulkanMesh.h"
#include <memory>
#include <map>

struct aiScene;

namespace render::backend::vulkan {
class VulkanMesh;
class VulkanTexture;
class Scene {
public:
    Scene(render::backend::Driver* driver);
    ~Scene();

    bool import(const char* path);
    void clear();

    inline uint32_t getNumNodes() const {return nodes.size();}
    inline const VulkanMesh* getNodeMesh(size_t index) const {return nodes[index].mesh;}
    inline const glm::mat4& getNodeWorldTransform(size_t index) const {return  nodes[index].transform;}
    inline const render::backend::BindSet* getNodeBindSet(size_t index) const
    {
        uint32_t material_index = nodes[index].render_material_index;
        return materials[material_index].bind_set;
    }


private:
    struct RenderMaterial{
        VulkanTexture* albedo{nullptr};
        VulkanTexture* normal{nullptr};
        VulkanTexture* roughness{nullptr};
        VulkanTexture* metallic{nullptr};
        render::backend::UniformBuffer* parameters{nullptr};
        render::backend::BindSet* bind_set{nullptr};
    };

    struct RenderNode{
        VulkanMesh* mesh;
        int32_t render_material_index{-1};
        glm::mat4 transform;
    };

private:
    void import_nodes(const aiScene* scene,const aiNode* root,const glm::mat4& transform);

private:
    render::backend::Driver* driver{nullptr};
    std::vector<VulkanMesh*> meshes;
    std::map<std::string,VulkanTexture*> textures;
    std::vector<RenderMaterial> materials;
    std::vector<RenderNode> nodes;
};
}

#endif //GAMEENGINE_SRC_BACKEND_VULKAN_SCENE_H
