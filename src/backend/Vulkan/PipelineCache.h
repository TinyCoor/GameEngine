//
// Created by 12132 on 2021/12/15.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_PIPELINECACHE_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_PIPELINECACHE_H
#include <unordered_map>
#include "driver.h"
#include <volk.h>
#include <cstdint>
namespace render::backend::vulkan {
class Device;
class context;
class DescriptorSetLayoutCache;
class PipelineLayoutCache;
class PipelineCache {
private:
    Device *device{nullptr};
    PipelineLayoutCache *pipeline_layout_cache{nullptr};
    std::unordered_map <uint64_t, VkPipeline> cache;
private:
    uint64_t getHash(VkPipelineLayout layout,const context *ctx,const RenderPrimitive* render_primitive) const;
public:
    PipelineCache(Device* device,PipelineLayoutCache *layout):device(device), pipeline_layout_cache(layout)
    {}
    ~PipelineCache();

    VkPipeline fetch(const context *ctx,const RenderPrimitive* render_primitive);
    void clear();
};

}

#endif //GAMEENGINE_SRC_BACKEND_VULKAN_PIPELINECACHE_H
