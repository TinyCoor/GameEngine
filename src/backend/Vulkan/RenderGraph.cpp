//
// Created by 12132 on 2021/12/25.
//

#include "RenderGraph.h"
#include "VulkanSwapChain.h"
#include "Scene.h"
#include "../../app/VulkanRenderScene.h"

render::backend::vulkan::RenderGraph::RenderGraph(render::backend::Driver *driver):driver(driver)
{
}

render::backend::vulkan::RenderGraph::~RenderGraph()
{

}
void render::backend::vulkan::RenderGraph::init(const VulkanRenderScene* scene, uint32_t width,uint32_t height)
{
    initGBuffer(width,height);
    initLBuffer(width,height);
    g_buffer_pass_vertex= scene->getGbufferVertexShader()->getShader();
    g_buffer_pass_fragment =scene->getGbufferFragmentShader()->getShader();
}

void render::backend::vulkan::RenderGraph::shutdown()
{
    shutdownGBuffer();
    shutdownLBuffer();
    g_buffer_pass_vertex= nullptr;
    g_buffer_pass_fragment= nullptr;
}

void render::backend::vulkan::RenderGraph::render(const Scene* scene,const VulkanRenderFrame& frame)
{
    renderGBuffer(scene,frame);
}

void render::backend::vulkan::RenderGraph::renderGBuffer(const Scene* scene,const VulkanRenderFrame& frame)
{
    RenderPassClearValue clear_values[4];
    memset(clear_values,0, sizeof(RenderPassClearValue)*4);
    RenderPassLoadOp load_ops[4];
    RenderPassStoreOp store_ops[4];

    for (int i = 0; i < 4; ++i) {
        load_ops[i] = RenderPassLoadOp::DONT_CARE;
        store_ops[i]= RenderPassStoreOp::STORE;
    }

    RenderPassInfo info;
    info.store_ops= store_ops;
    info.load_ops= load_ops;
    info.clear_value= clear_values;

    driver->beginRenderPass(frame.command_buffer,frame.frame_buffer,&info);
    driver->clearPushConstants();
    driver->clearBindSets();
    driver->allocateBindSets(2);

    driver->setBindSet(0,frame.bind_set);
    driver->setShader(ShaderType::VERTEX, g_buffer_pass_vertex);
    driver->setShader(ShaderType::FRAGMENT, g_buffer_pass_fragment);

    for (int i = 0; i < scene->getNumNodes(); ++i) {
        auto* node_mesh = scene->getNodeMesh(i);
        const auto& transform =  scene->getNodeWorldTransform(i);
        auto* node_bind_set = scene->getNodeBindSet(i);

        driver->setBindSet(1, node_bind_set);
        driver->setPushConstant(sizeof(glm::mat4), &transform);
        driver->drawIndexedPrimitive(frame.command_buffer,node_mesh->getPrimitive());
    }

    driver->endRenderPass(frame.command_buffer);
}
void render::backend::vulkan::RenderGraph::resize(uint32_t width, uint32_t height)
{
    shutdownGBuffer();
    shutdownLBuffer();

    initGBuffer(width,height);
    initLBuffer(width,height);
}
void render::backend::vulkan::RenderGraph::initGBuffer(uint32_t width,uint32_t height)
{
    g_buffer.base_color = driver->createTexture2D(width,height,1,Format::B8G8R8A8_UNORM);
    g_buffer.depth = driver->createTexture2D(width,height,1,Format::R32_SFLOAT);
    g_buffer.normal= driver->createTexture2D(width,height,1,Format::R16G16_SFLOAT);
    g_buffer.shading = driver->createTexture2D(width,height,1,Format::R8G8_UNORM);
    FrameBufferAttachment g_attachment[4] ={
        {FrameBufferAttachmentType::COLOR,g_buffer.base_color},
        {FrameBufferAttachmentType::COLOR,g_buffer.depth},
        {FrameBufferAttachmentType::COLOR,g_buffer.shading},
        {FrameBufferAttachmentType::COLOR,g_buffer.normal},
    };
    g_buffer.gbuffer = driver->createFrameBuffer(4,g_attachment);
}

void render::backend::vulkan::RenderGraph::shutdownGBuffer()
{
    driver->destroyTexture(g_buffer.base_color);
    driver->destroyTexture(g_buffer.depth);
    driver->destroyTexture(g_buffer.shading);
    driver->destroyTexture(g_buffer.normal);
    driver->destroyFrameBuffer(g_buffer.gbuffer);
    g_buffer = {};
}

void render::backend::vulkan::RenderGraph::initLBuffer(uint32_t width,uint32_t height)
{
    l_buffer.diffuse = driver->createTexture2D(width,height,1,Format::R16G16B16A16_SFLOAT);
    l_buffer.specular = driver->createTexture2D(width,height,1,Format::R16G16B16A16_SFLOAT);
    FrameBufferAttachment l_attachment[2] ={
        {FrameBufferAttachmentType::COLOR,l_buffer.diffuse},
        {FrameBufferAttachmentType::COLOR,l_buffer.specular},
    };

    l_buffer.lbuffer =  driver->createFrameBuffer(2,l_attachment);
}

void render::backend::vulkan::RenderGraph::shutdownLBuffer()
{
    driver->destroyTexture(l_buffer.diffuse);
    driver->destroyTexture(l_buffer.specular);
    driver->destroyFrameBuffer(l_buffer.lbuffer);
    l_buffer = {};
}
