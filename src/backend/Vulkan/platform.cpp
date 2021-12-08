//
// Created by 12132 on 2021/11/30.
//

#include "platform.h"
#include "VulkanContext.h"
#include <iostream>

#if defined(WIN32)
#include <windows.h>
#endif

void render::backend::vulkan::platform::destroySurface(const render::backend::vulkan::VulkanContext *context,
                                               const VkSurfaceKHR *surface) {
    vkDestroySurfaceKHR(context->Instance(),*surface, nullptr);
}

const char *render::backend::vulkan::platform::getInstanceExtension() {
#if defined(WIN32)
  return "VK_KHR_win32_surface";
#else
  #error "platform is not support"
#endif
  return nullptr;
}

VkSurfaceKHR render::backend::vulkan::platform::createSurface(const render::backend::vulkan::VulkanContext *context,
                                                              void *native_window) {
  VkSurfaceKHR surface{VK_NULL_HANDLE};
#if defined(WIN32)
  VkWin32SurfaceCreateInfoKHR surfaceInfo{};
  surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surfaceInfo.pNext= nullptr;
  surfaceInfo.hwnd = (HWND)(native_window);
  surfaceInfo.hinstance = GetModuleHandle(nullptr);

  if(vkCreateWin32SurfaceKHR(context->Instance(),&surfaceInfo,nullptr,&surface)!= VK_SUCCESS){
    std::cerr <<"create win32 surface failed" <<std::endl;
  }
#else
#error "Other platform not support"
#endif

  return surface;
}
