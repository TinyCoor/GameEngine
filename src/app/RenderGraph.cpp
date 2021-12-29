//
// Created by 12132 on 2021/12/25.
//

#include "RenderGraph.h"
#include "../backend/Vulkan/Scene.h"
#include "../backend/Vulkan/SkyLight.h"
#include "../backend/Vulkan/VulkanSwapChain.h"
#include "ApplicationResource.h"
#include <glm/gtc/matrix_transform.hpp>
namespace render::backend::vulkan {

RenderGraph::~RenderGraph()
{
}
void RenderGraph::init(const ApplicationResource *scene, uint32_t width, uint32_t height)
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
    renderLBuffer(scene,frame);
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

    driver->beginRenderPass(frame.command_buffer, g_buffer.frame_buffer, &info);
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

void RenderGraph::renderLBuffer(const Scene *scene, const VulkanRenderFrame &frame)
{
    RenderPassClearValue clear_values[2];
    memset(clear_values, 0, sizeof(RenderPassClearValue) * 2);

    RenderPassLoadOp load_ops[4] = { RenderPassLoadOp::DONT_CARE, RenderPassLoadOp::DONT_CARE, RenderPassLoadOp::DONT_CARE, RenderPassLoadOp::DONT_CARE };
    RenderPassStoreOp store_ops[4] = { RenderPassStoreOp::STORE, RenderPassStoreOp::STORE, RenderPassStoreOp::STORE, RenderPassStoreOp::STORE };

    RenderPassInfo info;
    info.clear_value = clear_values;
    info.load_ops = load_ops;
    info.store_ops = store_ops;

    driver->beginRenderPass(frame.command_buffer, l_buffer.frame_buffer, &info);

    driver->clearBindSets();
    driver->clearShaders();
    driver->clearPushConstants();
    driver->allocateBindSets(3);

    driver->setBindSet(0, frame.bind_set);
    driver->setBindSet(1, g_buffer.bindings);

    for (int i = 0; i < scene->getNumLights(); ++i) {
        const Light* light = scene->getLight(i);
        auto *node_mesh = scene->getNodeMesh(i);
        auto vert_shader = light->getVertexShader();
        auto frag_shader = light->getFragShader();
        auto bind_set = light->getBindSet();

        driver->clearShaders();
        driver->setShader(ShaderType::VERTEX, vert_shader->getShader());
        driver->setShader(ShaderType::FRAGMENT, frag_shader->getShader());

        driver->setBindSet(2, bind_set);
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

    g_buffer.frame_buffer = driver->createFrameBuffer(4, g_attachment);
    g_buffer.bindings = driver->createBindSet();
    driver->bindTexture(g_buffer.bindings,0,g_buffer.base_color);
    driver->bindTexture(g_buffer.bindings,1,g_buffer.depth);
    driver->bindTexture(g_buffer.bindings,2,g_buffer.normal);
    driver->bindTexture(g_buffer.bindings,3,g_buffer.shading);

}
void RenderGraph::initLBuffer(uint32_t width, uint32_t height)
{
    l_buffer.diffuse = driver->createTexture2D(width, height, 1, Format::R16G16B16A16_SFLOAT);
    l_buffer.specular = driver->createTexture2D(width, height, 1, Format::R16G16B16A16_SFLOAT);
    FrameBufferAttachment l_attachment[2] = {
        {FrameBufferAttachmentType::COLOR, l_buffer.diffuse},
        {FrameBufferAttachmentType::COLOR, l_buffer.specular},
    };

    l_buffer.frame_buffer = driver->createFrameBuffer(2, l_attachment);
    l_buffer.bindings= driver->createBindSet();
    driver->bindTexture(l_buffer.bindings,0,l_buffer.diffuse);
    driver->bindTexture(l_buffer.bindings,1,l_buffer.specular);

}
void RenderGraph::shutdownGBuffer()
{
    driver->destroyTexture(g_buffer.base_color);
    driver->destroyTexture(g_buffer.depth);
    driver->destroyTexture(g_buffer.shading);
    driver->destroyTexture(g_buffer.normal);
    driver->destroyFrameBuffer(g_buffer.frame_buffer);
    driver->destroyBindSet(g_buffer.bindings);
    memset(&g_buffer,0, sizeof(GBuffer));
}



void RenderGraph::shutdownLBuffer()
{
    driver->destroyTexture(l_buffer.diffuse);
    driver->destroyTexture(l_buffer.specular);
    driver->destroyFrameBuffer(l_buffer.frame_buffer);
    memset(&l_buffer,0, sizeof(GBuffer));
}
}
