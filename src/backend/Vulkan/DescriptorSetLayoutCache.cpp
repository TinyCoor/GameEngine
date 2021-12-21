//
// Created by 12132 on 2021/12/12.
//

#include "DescriptorSetLayoutCache.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "auxiliary.h"
namespace render::backend::vulkan {


uint64_t DescriptorSetLayoutCache::getHash(const BindSet *bind_set) const
{
    uint64_t hash  =0;

    for (int i =0 ;i <BindSet::MAX_BINDINGS; ++i ) {
        if(!bind_set->binding_used[i]){
            continue;
        }
        auto& info = bind_set->bindings[i];
        hash_combine(hash,info.binding);
        hash_combine(hash,info.stageFlags);
    }
    return hash;
}

DescriptorSetLayoutCache::DescriptorSetLayoutCache(const Device *device)
    : device(device)
{

}
DescriptorSetLayoutCache::~DescriptorSetLayoutCache()
{
    clear();
}

VkDescriptorSetLayout DescriptorSetLayoutCache::fetch(const BindSet *bind_set)
{
    uint64_t hash = getHash(bind_set);

    auto it = cache.find(hash);
    if(it != cache.end()){
        return it->second;
    }

    VulkanDescriptorSetLayoutBuilder builder;

    for (int i = 0; i <render::backend::vulkan::BindSet::MAX_BINDINGS ; ++i) {
        auto& info  = bind_set->bindings[i];
        if(!bind_set->binding_used[i]){
            continue;
        }
        builder.addDescriptorBinding(info.descriptorType,info.stageFlags,info.binding);
    }
    auto res  = builder.build(device->LogicDevice());
    cache[hash] = res;
    return res;
}

void DescriptorSetLayoutCache::clear()
{
    for (auto& item :cache) {
        vkDestroyDescriptorSetLayout(device->LogicDevice(),item.second, nullptr);
    }
    cache.clear();
}

}