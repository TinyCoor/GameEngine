//
// Created by 12132 on 2021/12/12.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_PIPELINELAYOUTCACHE_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_PIPELINELAYOUTCACHE_H
#include <unordered_map>
#include "../driver.h"
#include <volk.h>
#include <cstdint>
namespace render::backend::vulkan {
class Device;
class context;
class DescriptorSetLayoutCache;
class PipelineLayoutCache
{
private:
    Device* device{nullptr};
    DescriptorSetLayoutCache* descriptor_set_layout_cache{nullptr};
    std::unordered_map<uint64_t,VkPipelineLayout> cache;
private:
    uint64_t getHash(uint8_t num_layouts,const VkDescriptorSetLayout* layout,uint8_t push_constant_size) const;
public:
    PipelineLayoutCache(Device* device,DescriptorSetLayoutCache* layout)
        :device(device),descriptor_set_layout_cache(layout)
    {}
    ~PipelineLayoutCache();

    VkPipelineLayout fetch(const context* ctx);
    void clear();
};
}

#endif //GAMEENGINE_SRC_BACKEND_VULKAN_PIPELINELAYOUTCACHE_H
