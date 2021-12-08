//
// Created by 12132 on 2021/11/30.
//

#ifndef GAMEENGINE_SRC_PLATFORM_H
#define GAMEENGINE_SRC_PLATFORM_H
#include "driver.h"
#include <volk.h>
#include <vector>
namespace render::backend::vulkan {
class VulkanContext;
class platform {
public:
  static const char* getInstanceExtension();
  static VkSurfaceKHR createSurface(const VulkanContext* context, void* native_window);
  static void destroySurface(const VulkanContext* context, const VkSurfaceKHR * surface);
};

}

#endif //GAMEENGINE_SRC_PLATFORM_H