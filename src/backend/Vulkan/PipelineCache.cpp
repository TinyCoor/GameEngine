//
// Created by 12132 on 2021/12/15.
//

#include "PipelineCache.h"
#include "VulkanGraphicsPipelineBuilder.h"
#include "Utils.h"
#include "auxiliary.h"
#include "PipelineLayoutCache.h"
#include "context.h"

namespace render::backend::vulkan {

PipelineCache::~PipelineCache()
{
    clear();
}

void PipelineCache::clear()
{
    for (auto& item:cache) {
        vkDestroyPipeline(device->LogicDevice(),item.second, nullptr);
    }
    cache.clear();
}

uint64_t PipelineCache::getHash(VkPipelineLayout layout,const context *ctx, const RenderPrimitive *primitive) const
{
    uint64_t hash =0;
    hash_combine(hash,ctx->getRenderPass());
    hash_combine(hash,layout);
    auto vertex_buffer = static_cast<const render::backend::vulkan::VertexBuffer*>(primitive->vertex_buffer);

    for (int i = 0; i <vertex_buffer->num_attributes; ++i) {
        hash_combine(hash,vertex_buffer->attribute_formats[i]);
        hash_combine(hash,vertex_buffer->attribute_offsets[i]);
    }

    hash_combine(hash,primitive->topology);
    for (int i = 0; i <static_cast<uint32_t>(ShaderType::MAX); ++i) {
        auto module = ctx->getShader(static_cast<ShaderType>(i));
        if (module == VK_NULL_HANDLE) continue;
        hash_combine(hash,i);
        hash_combine(hash,module);
    }
    hash_combine(hash,ctx->getNumColorAttachment());
    hash_combine(hash,ctx->getMaxSampleCount());
    hash_combine(hash,ctx->getCullMode());
    hash_combine(hash,ctx->getDepthCompareFunc());
    hash_combine(hash,ctx->isDepthTestEnable());
    hash_combine(hash,ctx->isDepthWriteEnable());
    hash_combine(hash,ctx->isBlendingEnable());
    hash_combine(hash,ctx->getBlendSrcFactor());
    hash_combine(hash,ctx->getBlendDstFactor());
    return hash;
}

VkPipeline PipelineCache::fetch(const context *ctx, const RenderPrimitive *render_primitive)
{
    VkPipelineLayout layout = pipeline_layout_cache->fetch(ctx);
    uint64_t hash = getHash(layout,ctx,render_primitive);
    auto it = cache.find(hash);
    if(it != cache.end()){
        return it->second;
    }

    VulkanGraphicsPipelineBuilder builder(layout,ctx->getRenderPass());

    builder.addViewport(VkViewport());
    builder.addScissor(VkRect2D());
    builder.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
    builder.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    for (int i = 0; i <static_cast<uint32_t>(ShaderType::MAX); ++i) {
        auto module = ctx->getShader(static_cast<ShaderType>(i));
        if (module == VK_NULL_HANDLE) continue;
        builder.addShaderStage(module, toShaderStage(static_cast<ShaderType>(i)));
    }

    auto* vertex_buffer = static_cast<const vulkan::VertexBuffer*>(render_primitive->vertex_buffer);
    VkVertexInputBindingDescription vertex_input_binding{0,vertex_buffer->vertex_size,VK_VERTEX_INPUT_RATE_VERTEX };
    std::vector<VkVertexInputAttributeDescription> attributes(vertex_buffer->num_attributes);
    for (uint32_t i = 0; i <vertex_buffer->num_attributes ; ++i) {
        attributes[i] = {i,0, vertex_buffer->attribute_formats[i],vertex_buffer->attribute_offsets[i]};
    }

    builder.setInputAssemblyState(Utils::getPrimitiveTopology(render_primitive->topology));
    builder.addVertexInput(vertex_input_binding,attributes);

    builder.setRasterizerState(false, false,VK_POLYGON_MODE_FILL,1.0,ctx->getCullMode(),VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.setDepthStencilState(ctx->isDepthTestEnable(),ctx->isDepthWriteEnable(),ctx->getDepthCompareFunc());

    builder.setMultisampleState(ctx->getMaxSampleCount(), true);

    auto blend_enable = ctx->isBlendingEnable();
    auto src_factor = ctx->getBlendSrcFactor();
    auto dst_factor = ctx->getBlendDstFactor();
    for (int i = 0; i < ctx->getNumColorAttachment() ; ++i) {
        builder.addBlendColorAttachment(blend_enable,
                                        src_factor,
                                        dst_factor,
                                        VK_BLEND_OP_ADD,
                                        src_factor,
                                        dst_factor);

    }

    auto res =builder.build(device->LogicDevice());
    cache[hash] = res;
    return res;
}

}