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
class VulkanRenderScene;
class VulkanRenderScene;
struct VulkanRenderFrame;
}
using  namespace render::backend::vulkan;
class VulkanRender {
private:
    render::backend::Driver *driver{nullptr};
    VkExtent2D extent;

    VulkanCubeMapRender hdriToCubeRenderer;
    VulkanCubeMapRender diffuseIrradianceRenderer;
    std::vector<VulkanCubeMapRender *> cubeToPrefilteredRenderers;
    VulkanTexture2DRender brdfRender;

    VulkanTexture brdfBaked;
    VulkanTexture environmentCubemap;
    VulkanTexture diffuseIrradianceCubemap;

    render::backend::BindSet *scene_bind_set{nullptr};

public:
    explicit VulkanRender(render::backend::Driver *driver,
                          VkExtent2D extent);
    virtual ~VulkanRender();

    void setextent(int width, int height)
    {
        extent.width = width;
        extent.height = height;
    }

    void init(VulkanRenderScene *scene);

    void update(RenderState &state, VulkanRenderScene *scene);

    void render(VulkanRenderScene *scene, const VulkanRenderFrame &frame);

    void shutdown();

    void resize(const VulkanSwapChain *swapChain);

    void setEnvironment(const VulkanRenderScene *scene,const VulkanTexture *texture);

private:

};


#endif //GAMEENGINE_VULKANRENDER_H
