//
// Created by 12132 on 2021/12/12.
//

#include "DescriptorSetCache.h"
#include "Device.h"
#include "DescriptorSetLayoutCache.h"
#include "auxiliary.h"
namespace render::backend::vulkan {

uint64_t DescriptorSetCache::getHash(const BindSet *bind_set) const
{
    uint64_t hash  =0;

    for (int i =0 ;i <BindSet::MAX_BINDINGS; ++i ) {
        if(!bind_set->bind_used[i]){
            continue;
        }
        auto& info = bind_set->bindings[i];
        auto& data = bind_set->bind_data[i];

        hash_combine(hash,info.descriptorType);
        hash_combine(hash,info.binding);
        hash_combine(hash,info.descriptorCount);
        hash_combine(hash,info.stageFlags);
        if(info.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER){
            hash_combine(hash,data.texture.view);
            hash_combine(hash,data.texture.sampler);
        } else{
            hash_combine(hash,data.ubo);
        }
    }
    return hash;
}

DescriptorSetCache::DescriptorSetCache(const Device *device, DescriptorSetLayoutCache* layout)
    : device(device),layout(layout)
{

}
DescriptorSetCache::~DescriptorSetCache()
{
    clear();
}

VkDescriptorSet DescriptorSetCache::fetch(const BindSet *bind_set)
{
    uint64_t hash = getHash(bind_set);

    auto it = cache.find(hash);
    if(it != cache.end()){
        return it->second;
    }
    auto set_layout = layout->fetch(bind_set);
    VkDescriptorSetAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorPool= device->DescriptorPool();
    info.pSetLayouts= &set_layout;
    info.descriptorSetCount =1;

    VkDescriptorSet descriptor_set{VK_NULL_HANDLE};
    vkAllocateDescriptorSets(device->LogicDevice(),&info,&descriptor_set);
    //TODO
    cache[hash]= descriptor_set;
    return descriptor_set;
}

void DescriptorSetCache::clear()
{
    for (auto& item :cache) {
        vkFreeDescriptorSets(device->LogicDevice(),device->DescriptorPool(),1,&item.second);
    }
    cache.clear();
}

}