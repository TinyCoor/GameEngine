//
// Created by 12132 on 2021/12/25.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_RENDERGRAPH_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_RENDERGRAPH_H

#include "../backend/Vulkan/driver.h"

namespace render::backend::vulkan{

struct GBuffer{
    render::backend::Texture* base_color{nullptr};                  ///
    render::backend::Texture* depth{nullptr};                       ///
    render::backend::Texture* shading{nullptr};                     ///
    render::backend::Texture* normal{nullptr};                      ///
    render::backend::FrameBuffer* frame_buffer{nullptr};            ///
    render::backend::BindSet* bindings{nullptr};
};

struct LBuffer{
    render::backend::Texture* diffuse{nullptr};
    render::backend::Texture* specular{nullptr};
    render::backend::FrameBuffer* frame_buffer{nullptr};
    render::backend::BindSet* bindings{nullptr};
};

class Scene;
struct VulkanRenderFrame;
class ApplicationResource;

class RenderGraph {
public:
    RenderGraph(render::backend::Driver* driver) : driver(driver){}
    ~RenderGraph();

    void init(const ApplicationResource* scene,uint32_t width,uint32_t height);
    void resize(uint32_t width,uint32_t height);
    void shutdown();
    void render(const Scene* scene,const VulkanRenderFrame& frame);

    inline GBuffer& getGBuffer() {return g_buffer;}
    inline LBuffer& getLBuffer() {return l_buffer;}
private:
    void renderGBuffer(const Scene* scene,const VulkanRenderFrame& frame);
    void renderLBuffer(const Scene* scene,const VulkanRenderFrame& frame);
    void initGBuffer(uint32_t width,uint32_t height);
    void shutdownGBuffer();

    void initLBuffer(uint32_t width,uint32_t height);
    void shutdownLBuffer();

private:
    render::backend::Driver* driver;

    GBuffer g_buffer;
    LBuffer l_buffer;

    render::backend::Shader* g_buffer_pass_vertex{nullptr};
    render::backend::Shader* g_buffer_pass_fragment{nullptr};

};
}


#endif //GAMEENGINE_SRC_BACKEND_VULKAN_RENDERGRAPH_H
