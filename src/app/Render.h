//
// Created by y123456 on 2021/10/10.
//

#ifndef GAMEENGINE_VULKANRENDER_H
#define GAMEENGINE_VULKANRENDER_H

#include "../backend/Vulkan/VulkanCubemapRender.h"
#include "../backend/Vulkan/VulkanTexture2DRender.h"
#include <volk.h>
#include <vector>


namespace render::backend::vulkan {
struct RenderState;
class Device;
class VulkanSwapChain;
class SkyLight;
class ApplicationResource;
struct VulkanRenderFrame;
}
using  namespace render::backend::vulkan;
class Render {
private:
    render::backend::Driver *driver{nullptr};
    render::backend::BindSet *scene_bind_set{nullptr};

public:
    explicit Render(render::backend::Driver *driver);
    virtual ~Render();

    void init(ApplicationResource *scene);

    void render(const ApplicationResource *scene,const SkyLight *light, const VulkanRenderFrame &frame);

    void shutdown();

    void setEnvironment(const ApplicationResource *resource,uint8_t index);


private:

};


#endif //GAMEENGINE_VULKANRENDER_H
