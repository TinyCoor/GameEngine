//
// Created by 12132 on 2021/12/12.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_DESCRIPTORSETLYOUTCACHE_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_DESCRIPTORSETLYOUTCACHE_H
#include <unordered_map>
#include "driver.h"
#include <volk.h>
#include <cstdint>
namespace render::backend::vulkan {
class Device;

class DescriptorSetLayoutCache
{
private:
    const Device* device{nullptr};
    std::unordered_map<uint64_t,VkDescriptorSetLayout> cache;

private:
    uint64_t getHash(const BindSet* bind_set) const;
public:
    DescriptorSetLayoutCache(const Device* device);
    ~DescriptorSetLayoutCache();

    VkDescriptorSetLayout fetch(const BindSet* bind_set);
    void clear();

};
}

#endif //GAMEENGINE_SRC_BACKEND_VULKAN_DESCRIPTORSETCACHE_H
