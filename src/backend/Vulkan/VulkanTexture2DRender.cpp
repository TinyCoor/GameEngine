//
// Created by 12132 on 2021/11/9.
//

#include "VulkanTexture2DRender.h"
#include "VulkanTexture.h"
#include "VulkanShader.h"

using namespace render::backend::vulkan;
VulkanTexture2DRender::VulkanTexture2DRender(render::backend::Driver *driver)
    : driver(driver), quad(driver)
{
}

VulkanTexture2DRender::~VulkanTexture2DRender()
{
    shutdown();
}

void VulkanTexture2DRender::init(const VulkanTexture* target_texture)
{
    quad.createQuad(2.0f);
    // Create framebuffer
    render::backend::FrameBufferAttachment attachments={
        FrameBufferAttachmentType::COLOR, target_texture->getTexture(), 0, 1, 0, 1,
    };
    framebuffer = driver->createFrameBuffer(1, &attachments);

    // Create command buffers
    command_buffer = driver->createCommandBuffer(CommandBufferType::PRIMARY);

}

void VulkanTexture2DRender::shutdown()
{
    quad.clearGPUData();
    quad.clearCPUData();

    driver->destroyFrameBuffer(framebuffer);
    framebuffer= nullptr;
    driver->destroyCommandBuffer(command_buffer);
    command_buffer= nullptr;
}

void VulkanTexture2DRender::render(const VulkanShader* vertex_shader,const VulkanShader* fragment_shader)
{
    // Record command buffers
    driver->resetCommandBuffer(command_buffer);
    driver->beginCommandBuffer(command_buffer);

    RenderPassClearValue clear_value;
    clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};

    RenderPassLoadOp load_op = RenderPassLoadOp::CLEAR;
    RenderPassStoreOp store_op = RenderPassStoreOp::STORE;

    RenderPassInfo info;
    info.load_ops = &load_op;
    info.store_ops = &store_op;
    info.clear_value = &clear_value;

    driver->beginRenderPass(command_buffer, framebuffer, &info);

    driver->clearShaders();
    driver->setShader(ShaderType::VERTEX, vertex_shader->getShader());
    driver->setShader(ShaderType::FRAGMENT, fragment_shader->getShader());

    driver->drawIndexedPrimitive(command_buffer, quad.getPrimitive());

    driver->endRenderPass(command_buffer);

    driver->endCommandBuffer(command_buffer);
    driver->submit(command_buffer);
    driver->wait(1, &command_buffer);

}
