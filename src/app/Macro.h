//
// Created by y123456 on 2021/10/11.
//

#ifndef GAMEENGINE_MACRO_H
#define GAMEENGINE_MACRO_H
#include <vulkan.h>
#include <stdexcept>

#define OFFSET_OF(type,member) ((size_t) (& ((type*) 0)->member))

#define VK_CHECK(call,msg)                              \
    do {                                                \
        VkResult res  =(call);                            \
        if(res != VK_SUCCESS ){                          \
             throw std::runtime_error(msg);               \
        }                                                 \
    }while(0)

#define TH_WITH_MSG(condition,msg) \
    do{                               \
    if((condition))                  \
        throw std::runtime_error(msg);  \
    }while(0)


#define CERR_MSG(condition,msg) \
    do{                               \
    if((condition))                  \
       std::cout << msg ;       \
       return false;             \
    }while(0)

#define VK_DESTROY_OBJECT(destructor_call,variable) \
                do{                                 \
                 (destructor_call) ;             \
                 variable =VK_NULL_HANDLE;                \
                }while(0)

#define VK_INIT_VARIABLE_OBJECT(type,var_name) \
                type var_name = VK_NULL_HANDLE

#define FOR(i,index) \
    for(i =0; i< index; i++)

#endif //GAMEENGINE_MACRO_H

