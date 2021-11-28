//
// Created by 12132 on 2021/11/28.
//
#include "Vulkan/driver.h"
#include "API.h"

namespace render::backend{

 Driver* Driver::create(const char* app_name,const char* engine_name, Api api)
 {
   switch (api) {
      case Api::VULKAN: return new vulkan::VulkanDriver(app_name,engine_name);
   }
   return nullptr;
 }
}