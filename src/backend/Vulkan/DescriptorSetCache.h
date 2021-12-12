//
// Created by 12132 on 2021/12/12.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_DESCRIPTORSETCACHE_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_DESCRIPTORSETCACHE_H
#include <unordered_map>
#include "driver.h"
#include <volk.h>
#include <cstdint>
namespace render::backend::vulkan {
class Device;
class DescriptorSetLayoutCache;
class DescriptorSetCache
{
private:
    const Device* device{nullptr};
    DescriptorSetLayoutCache* layout;
    std::unordered_map<uint64_t,VkDescriptorSet> cache;

private:
    uint64_t getHash(const BindSet* bind_set) const;
public:
    DescriptorSetCache(const Device* device, DescriptorSetLayoutCache* layout);
    ~DescriptorSetCache();

    VkDescriptorSet fetch(const BindSet* bind_set);
    void clear();

};
}

#endif //GAMEENGINE_SRC_BACKEND_VULKAN_DESCRIPTORSETCACHE_H
