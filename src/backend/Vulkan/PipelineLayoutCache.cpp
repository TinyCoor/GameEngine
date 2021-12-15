//
// Created by 12132 on 2021/12/12.
//

#include "PipelineLayoutCache.h"
#include "context.h"
#include "DescriptorSetLayoutCache.h"
#include "VulkanPipelineLayoutBuilder.h"
#include "auxiliary.h"
#include <vector>

namespace render::backend::vulkan{

uint64_t PipelineLayoutCache::getHash(const uint8_t num_layouts,const VkDescriptorSetLayout* layouts) const
{
    uint64_t hash  =0;
    hash_combine(hash,num_layouts);
    for (uint8_t i = 0; i < num_layouts; ++i){
        hash_combine(hash,layouts[i]);
    }
    return hash;
}

PipelineLayoutCache::~PipelineLayoutCache()
{
    clear();
}
VkPipelineLayout PipelineLayoutCache::fetch(const context* ctx)
{
    std::vector<VkDescriptorSetLayout> layouts(ctx->getNumBindSets());
    uint8_t index =0;
    for (auto& layout :layouts) {
        layout = descriptor_set_layout_cache->fetch(ctx->getBindSet(index));
    }
    uint64_t hash = getHash(layouts.size(),layouts.data());
    auto it = cache.find(hash);
    if(it != cache.end()){
        return it->second;
    }

    VulkanPipelineLayoutBuilder builder;
    for (auto& layout :layouts) {
        builder.addDescriptorSetLayout(layout);
    }

    /// TODO push constant
    auto res =  builder.build(device->LogicDevice());
    cache[hash]= res;

    return res;
}

void PipelineLayoutCache::clear()
{
    for (auto& item : cache) {
        vkDestroyPipelineLayout(device->LogicDevice(),item.second, nullptr);
    }
    cache.clear();
}
}
