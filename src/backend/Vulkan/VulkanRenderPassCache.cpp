//
// Created by 12132 on 2021/12/11.
//

#include "VulkanRenderPassCache.h"
#include "Device.h"
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
    return nullptr;
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
//    hash_combine(hash,frame_buffer->num_attachments);
//
//    for (uint32_t i = 0; i < frame_buffer->num_attachments ; ++i) {
//        hash_combine(hash,frame_buffer->attachment_format[i]);
//        hash_combine(hash,frame_buffer->attachment_samples[i]);
//        hash_combine(hash,frame_buffer->attachment_resolve[i]);
//        hash_combine(hash,info->load_ops[i]);
//        hash_combine(hash,info->store_ops[i]);
//    }
    return hash;
}
}