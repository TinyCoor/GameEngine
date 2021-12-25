//
// Created by 12132 on 2021/12/25.
//

#include <glm/gtc/matrix_transform.hpp>
#include "RenderGraph.h"
#include "VulkanSwapChain.h"
#include "Scene.h"
#include "../../app/VulkanRenderScene.h"
namespace render::backend::vulkan {



RenderGraph::~RenderGraph()
{

}
void RenderGraph::init(const VulkanRenderScene *scene, uint32_t width, uint32_t height)
{
    initGBuffer(width, height);
    initLBuffer(width, height);
    g_buffer_pass_vertex = scene->getGbufferVertexShader()->getShader();
    g_buffer_pass_fragment = scene->getGbufferFragmentShader()->getShader();
}

void RenderGraph::shutdown()
{
    shutdownGBuffer();
    shutdownLBuffer();
    g_buffer_pass_vertex = nullptr;
    g_buffer_pass_fragment = nullptr;
}

void RenderGraph::render(const Scene *scene, const VulkanRenderFrame &frame)
{
    renderGBuffer(scene, frame);
}

void RenderGraph::renderGBuffer(const Scene *scene, const VulkanRenderFrame &frame)
{
    RenderPassClearValue clear_values[4];
    memset(clear_values, 0, sizeof(RenderPassClearValue) * 4);

    RenderPassLoadOp load_ops[4] = { RenderPassLoadOp::DONT_CARE, RenderPassLoadOp::DONT_CARE, RenderPassLoadOp::DONT_CARE, RenderPassLoadOp::DONT_CARE };
    RenderPassStoreOp store_ops[4] = { RenderPassStoreOp::STORE, RenderPassStoreOp::STORE, RenderPassStoreOp::STORE, RenderPassStoreOp::STORE };

    RenderPassInfo info;
    info.clear_value = clear_values;
    info.load_ops = load_ops;
    info.store_ops = store_ops;

    driver->beginRenderPass(frame.command_buffer, frame.frame_buffer, &info);
    driver->clearPushConstants();
    driver->clearBindSets();
    driver->allocateBindSets(2);

    driver->setBindSet(0, frame.bind_set);
    driver->setShader(ShaderType::VERTEX, g_buffer_pass_vertex);
    driver->setShader(ShaderType::FRAGMENT, g_buffer_pass_fragment);
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0), glm::radians(90.f), glm::vec3(1.0, 0.0, 0.0));

    for (int i = 0; i < scene->getNumNodes(); ++i) {
        auto *node_mesh = scene->getNodeMesh(i);
        const auto &transform = rotation * scene->getNodeWorldTransform(i);
        auto *node_bind_set = scene->getNodeBindSet(i);

        driver->setBindSet(1, node_bind_set);
        driver->setPushConstant(sizeof(glm::mat4), &transform);
        driver->drawIndexedPrimitive(frame.command_buffer, node_mesh->getPrimitive());
    }

    driver->endRenderPass(frame.command_buffer);
}
void RenderGraph::resize(uint32_t width, uint32_t height)
{
    shutdownGBuffer();
    shutdownLBuffer();

    initGBuffer(width, height);
    initLBuffer(width, height);
}
void RenderGraph::initGBuffer(uint32_t width, uint32_t height)
{
    g_buffer.base_color = driver->createTexture2D(width, height, 1, Format::B8G8R8A8_UNORM);
    g_buffer.depth = driver->createTexture2D(width, height, 1, Format::R32_SFLOAT);
    g_buffer.normal = driver->createTexture2D(width, height, 1, Format::R16G16_SFLOAT);
    g_buffer.shading = driver->createTexture2D(width, height, 1, Format::R8G8_UNORM);
    FrameBufferAttachment g_attachment[4] = {
        {FrameBufferAttachmentType::COLOR, g_buffer.base_color},
        {FrameBufferAttachmentType::COLOR, g_buffer.depth},
        {FrameBufferAttachmentType::COLOR, g_buffer.shading},
        {FrameBufferAttachmentType::COLOR, g_buffer.normal},
    };
    g_buffer.gbuffer = driver->createFrameBuffer(4, g_attachment);
}

void RenderGraph::shutdownGBuffer()
{
    driver->destroyTexture(g_buffer.base_color);
    driver->destroyTexture(g_buffer.depth);
    driver->destroyTexture(g_buffer.shading);
    driver->destroyTexture(g_buffer.normal);
    driver->destroyFrameBuffer(g_buffer.gbuffer);
    g_buffer = {};
}

void RenderGraph::initLBuffer(uint32_t width, uint32_t height)
{
    l_buffer.diffuse = driver->createTexture2D(width, height, 1, Format::R16G16B16A16_SFLOAT);
    l_buffer.specular = driver->createTexture2D(width, height, 1, Format::R16G16B16A16_SFLOAT);
    FrameBufferAttachment l_attachment[2] = {
        {FrameBufferAttachmentType::COLOR, l_buffer.diffuse},
        {FrameBufferAttachmentType::COLOR, l_buffer.specular},
    };

    l_buffer.lbuffer = driver->createFrameBuffer(2, l_attachment);
}

void RenderGraph::shutdownLBuffer()
{
    driver->destroyTexture(l_buffer.diffuse);
    driver->destroyTexture(l_buffer.specular);
    driver->destroyFrameBuffer(l_buffer.lbuffer);
    l_buffer = {};
}
}
