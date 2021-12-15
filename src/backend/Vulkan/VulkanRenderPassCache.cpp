//
// Created by 12132 on 2021/12/11.
//

#include "VulkanRenderPassCache.h"
#include "VulkanRenderPassBuilder.h"
#include "auxiliary.h"
namespace render::backend::vulkan {

VulkanRenderPassCache::~VulkanRenderPassCache()
{
    clear();
}

VkRenderPass VulkanRenderPassCache::fetch(const render::backend::vulkan::FrameBuffer *frame_buffer,
                                                                   const render::backend::RenderPassInfo *info)
{
    uint64_t hash  = getHash(frame_buffer,info);
    auto it= cache.find(hash);
    if (it != cache.end()) {
        return it->second;
    }
    VulkanRenderPassBuilder builder;
    builder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS);

    for (uint32_t i = 0; i <frame_buffer->num_attachments ; ++i) {
        VkFormat format = frame_buffer->attachment_format[i];
        FrameBufferAttachmentType type = frame_buffer->attachment_types[i];
        VkAttachmentLoadOp load_op = static_cast<VkAttachmentLoadOp>(info->load_ops[i]);
        VkSampleCountFlagBits samples = frame_buffer->attachment_samples[i];
        VkAttachmentStoreOp store_op = static_cast<VkAttachmentStoreOp>(info->store_ops[i]);
        bool resolve = frame_buffer->attachment_resolve[i];
        if(type == FrameBufferAttachmentType::DEPTH){
            builder.addDepthStencilAttachment(format,samples);
            builder.setDepthStencilAttachmentReference(0,i);
        } else if( resolve){
            builder.addColorResolveAttachment(format,load_op,store_op);
            builder.addColorResolveAttachmentReference(0,i);
        }else {
            builder.addColorAttachment(format,samples,load_op,store_op);
            builder.addColorAttachmentReference(0,i);
        }
    }
    VkRenderPass result = builder.build(device->LogicDevice());
    cache.insert(std::make_pair(hash,result));
    return result;
}
void VulkanRenderPassCache::clear()
{
    for (auto& item : cache){
        vkDestroyRenderPass(device->LogicDevice(),item.second, nullptr);
    }
    cache.clear();
}
uint64_t VulkanRenderPassCache::getHash(const FrameBuffer *frame_buffer, const RenderPassInfo *info) const
{
    uint64_t hash =0;
    hash_combine(hash,frame_buffer->num_attachments);

    for (uint32_t i = 0; i < frame_buffer->num_attachments ; ++i) {
        hash_combine(hash,frame_buffer->attachment_format[i]);
        hash_combine(hash,frame_buffer->attachment_samples[i]);
        hash_combine(hash,frame_buffer->attachment_resolve[i]);
        hash_combine(hash,info->load_ops[i]);
        hash_combine(hash,info->store_ops[i]);
    }
    return hash;
}
}