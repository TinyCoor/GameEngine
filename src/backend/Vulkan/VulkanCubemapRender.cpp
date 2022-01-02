//
// Created by y123456 on 2021/10/10.
//

#include <glm/gtc/matrix_transform.hpp>
#include "VulkanCubemapRender.h"
#include "VulkanGraphicsPipelineBuilder.h"
namespace render::backend::vulkan {

struct CubemapFaceOrientationData {
    glm::mat4 faces[6];
};

VulkanCubeMapRender::VulkanCubeMapRender(render::backend::Driver *driver)
    : driver(driver),quad(driver)
{
}

VulkanCubeMapRender::~VulkanCubeMapRender(){
    shutdown();
}

void VulkanCubeMapRender::init(VulkanTexture* target_texture,int target_mip)
{
    quad.createQuad(2.0f);

    uint32_t ubo_size = sizeof(CubemapFaceOrientationData);
    uniform_buffer = driver->createUniformBuffer(render::backend::BufferType::DYNAMIC, ubo_size);

    bind_set = driver->createBindSet();

    //create frameBuffer
    render::backend::FrameBufferAttachment attachments[6] =
    {
        {FrameBufferAttachmentType::COLOR, target_texture->getTexture(), target_mip, 1, 0, 1},
        {FrameBufferAttachmentType::COLOR, target_texture->getTexture(), target_mip, 1, 1, 1},
        {FrameBufferAttachmentType::COLOR, target_texture->getTexture(), target_mip, 1, 2, 1},
        {FrameBufferAttachmentType::COLOR, target_texture->getTexture(), target_mip, 1, 3, 1},
        {FrameBufferAttachmentType::COLOR, target_texture->getTexture(), target_mip, 1, 4, 1},
        {FrameBufferAttachmentType::COLOR, target_texture->getTexture(), target_mip, 1, 5, 1},
    };
    framebuffer = driver->createFrameBuffer(6, attachments);

    // Create command buffers
    command_buffer = driver->createCommandBuffer(CommandBufferType::PRIMARY);

    //fill uniform buffer
    auto *ubo = reinterpret_cast<CubemapFaceOrientationData *>(driver->map(uniform_buffer));

    const glm::mat4 &translateZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    const glm::vec3 faceDirs[6] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
    };

    const glm::vec3 faceUps[6] = {
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
    };

    const glm::mat4 faceRotations[6] = {
        glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    };
    for (int i = 0; i < 6; i++)
        ubo->faces[i] = faceRotations[i] * glm::lookAtRH(glm::vec3(0.0f), faceDirs[i], faceUps[i]) * translateZ;

    driver->unmap(uniform_buffer);

    driver->bindUniformBuffer(bind_set,0,uniform_buffer);

}

void VulkanCubeMapRender::shutdown()
{
    driver->destroyUniformBuffer(uniform_buffer);
    uniform_buffer = nullptr;

    driver->destroyFrameBuffer(framebuffer);
    framebuffer = nullptr;

    driver->destroyCommandBuffer(command_buffer);
    command_buffer = nullptr;

    driver->destroyBindSet(bind_set);
    bind_set= nullptr;

    quad.clearGPUData();
    quad.clearCPUData();

}

void VulkanCubeMapRender::render(const VulkanShader* vertShader,
                                 const VulkanShader* fragShader,
                                 const VulkanTexture* inputTexture,
                                 int input_mip,
                                 uint8_t push_constant_size,
                                 const void* data)
{
    if(input_mip == -1 )
    {
        driver->bindTexture(bind_set,1,inputTexture->getTexture());
    } else {
        driver->bindTexture(bind_set,1,inputTexture->getTexture(),input_mip,1,0,inputTexture->getNumLayers());
    }

    RenderPassClearValue clear_values[6];
    RenderPassLoadOp load_ops[6] ;
    RenderPassStoreOp store_ops[6];
    for (int i = 0; i < 6 ; ++i) {
        clear_values[i].color = {0.0f, 0.0f, 0.0f, 1.0f};
        load_ops[i] = RenderPassLoadOp::CLEAR;
        store_ops[i]= RenderPassStoreOp::STORE;
    }

    RenderPassInfo info;
    info.load_ops = load_ops;
    info.store_ops = store_ops;
    info.clear_value = clear_values;

    driver->clearPushConstants();
    if (push_constant_size > 0)
        driver->setPushConstant(push_constant_size,data);
    driver->clearShaders();
    driver->clearBindSets();
    driver->pushBindSet(bind_set);
    // Record command buffers
    driver->resetCommandBuffer(command_buffer);
    driver->beginCommandBuffer(command_buffer);

    driver->setShader(ShaderType::VERTEX,vertShader->getShader());
    driver->setShader(ShaderType::FRAGMENT,fragShader->getShader());

    driver->beginRenderPass(command_buffer,framebuffer,&info);

    //todo draw after clear state
    driver->drawIndexedPrimitive(command_buffer,quad.getPrimitive());
    driver->endRenderPass(command_buffer);
    driver->endCommandBuffer(command_buffer);
    driver->submit(command_buffer);
    driver->wait(1,&command_buffer);
}
}
