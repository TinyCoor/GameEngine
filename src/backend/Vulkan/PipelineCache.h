//
// Created by 12132 on 2021/12/12.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_PIPELINECACHE_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_PIPELINECACHE_H
#include <unordered_map>
#include "../API.h"
#include <volk.h>
#include <cstdint>
namespace render::backend::vulkan {
class Device;
class PipelineCache
{
private:
    Device* device{nullptr};
    std::unordered_map<uint64_t,VkPipeline> cache;
private:
    uint64_t getHash(const FrameBuffer* frame_buffer,const RenderPassInfo* info) const;
public:
    PipelineCache(Device* device):device(device){}
    ~PipelineCache();

    VkPipeline fetch(const FrameBuffer* frame_buffer,const RenderPassInfo* info);
    void clear();
};
}

#endif //GAMEENGINE_SRC_BACKEND_VULKAN_PIPELINECACHE_H
