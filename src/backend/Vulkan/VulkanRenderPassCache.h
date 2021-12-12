//
// Created by 12132 on 2021/12/11.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_VULKANRENDERPASSCACHE_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_VULKANRENDERPASSCACHE_H
#include <unordered_map>
#include <cstdint>
#include <volk.h>
#include "driver.h"
namespace render::backend::vulkan {
class VulkanRenderPassCache
{
private:
    Device* device{nullptr};
    std::unordered_map<uint64_t,VkRenderPass> cache;
private:
    uint64_t getHash(const FrameBuffer* frame_buffer,const RenderPassInfo* info) const;
public:
    VulkanRenderPassCache(Device* device):device(device){}
    ~VulkanRenderPassCache();

    VkRenderPass fetch(const FrameBuffer* frame_buffer,const RenderPassInfo* info);
    void clear();

};
}

#endif //GAMEENGINE_SRC_BACKEND_VULKAN_VULKANRENDERPASSCACHE_H
