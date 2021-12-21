//
// Created by 12132 on 2021/12/14.
//

#include <cassert>
#include "context.h"
namespace render::backend::vulkan {
context::context()
{
    clear();
}

context::~context()
{
    clear();
}

void context::setShader(ShaderType type, const Shader *shader)
{
    shaders[static_cast<uint32_t>(type)] = static_cast<const vulkan::Shader *>(shader)->shaderModule;
}

void context::clear()
{
    clearShaders();
    clearBindSets();
    current_pass = VK_NULL_HANDLE;
    state = {};
    state.cull_mode = VK_CULL_MODE_BACK_BIT;
    state.depth_test = 1;
    state.blending = 1;
    state.depth_write = 1;
    state.depth_compare_func = VK_COMPARE_OP_LESS;
    viewport = {};
    scissor = {};
}

void context::clearBindSets()
{
    for (uint32_t i = 0; i < MAX_SET; ++i) {
        set[i] = {};
    }
    num_sets = 0;
}

void context::clearShaders()
{
    for (int i = 0; i < static_cast<uint32_t>(ShaderType::MAX); ++i) {
        shaders[i] = VK_NULL_HANDLE;
    }
}

void context::pushBindSet(BindSet *bind_set)
{
    //todo log
    assert(num_sets < MAX_SET);
    assert(bind_set);
    set[num_sets++] = bind_set;
}

void context::setBindSet(uint8_t binding, BindSet *bind_set)
{
    assert(binding < MAX_SET);
    assert(bind_set);
    set[binding] = bind_set;
}
void context::setFrameBuffer(const FrameBuffer *frame_buffer)
{
    samples = VK_SAMPLE_COUNT_1_BIT;
    num_color_attachment = 0;

    if (frame_buffer == nullptr)
        return;

    for (uint8_t index = 0; index < frame_buffer->num_attachments; ++index) {
        if (frame_buffer->attachment_types[index] == FrameBufferAttachmentType::DEPTH) {
            continue;
        }
        if (frame_buffer->attachment_resolve[index])
            continue;
        num_color_attachment++;
        samples = std::max(samples, frame_buffer->attachment_samples[index]);
    }
    viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(frame_buffer->sizes.width);
    viewport.height = static_cast<float>(frame_buffer->sizes.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = frame_buffer->sizes;
}

}