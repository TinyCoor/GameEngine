//
// Created by ygp on 2021/9/14.
//

/**
 * This file is define the macro that other source file
 * would used
 */

#ifndef GAMEENGINE_MACRO_H
#define GAMEENGINE_MACRO_H

#ifdef __cplusplus
#include <vulkan.hpp>
#else
#include <vulkan.h>
#endif


#define VK_CHECK(call)                                  \
    do {                                                \
        VkResult result  =call;                         \
        assert(result == VK_SUCCESS );                  \
    }while(0);



#endif //GAMEENGINE_MACRO_H
