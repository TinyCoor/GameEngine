//
// Created by 12132 on 2021/12/15.
//

#ifndef GAMEENGINE_SRC_BACKEND_VULKAN_VULKANRESOURCECACHE_H
#define GAMEENGINE_SRC_BACKEND_VULKAN_VULKANRESOURCECACHE_H
#include <unordered_map>
#include "../API.h"
#include "Device.h"
#include <volk.h>
#include <cstdint>
namespace render::backend::vulkan {
//TODO
template<typename vk_resource,typename DependencyType>
class VulkanResourceCache {
public:

private:

private:
    Device* device{nullptr};
    DependencyType* dependent{nullptr};
    std::unordered_map<uint64_t ,vk_resource> cache;
};

}
#endif //GAMEENGINE_SRC_BACKEND_VULKAN_VULKANRESOURCECACHE_H
