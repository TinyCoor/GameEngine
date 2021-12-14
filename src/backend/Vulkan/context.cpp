//
// Created by 12132 on 2021/12/14.
//

#include <cassert>
#include "context.h"
namespace render::backend::vulkan{
context::context()
{
    clear();
}

context::~context(){
    clear();
}


void context::setShader(ShaderType type, const Shader *shader)
{
    shaders[static_cast<uint32_t>(type)] = static_cast<const vulkan::Shader*>(shader)->shaderModule;
}

void context::clear()
{
    clearShaders();
    clearBindSets();
    current_pass= VK_NULL_HANDLE;
    state = {};
}

void context::clearBindSets()
{
    for (uint32_t i= 0; i < MAX_SET; ++i){
        set[i] = {};
    }
}
void context::clearShaders()
{
    for (int i = 0; i < static_cast<uint32_t>(ShaderType::MAX) ; ++i) {
         shaders[i] = VK_NULL_HANDLE;
    }
}

void context::pushBindSet(const BindSet *bind_set)
{
    //todo log
    assert(num_sets < MAX_SET);
    assert(bind_set);
    set[num_sets++] = *bind_set;
}

void context::setBindSet(uint8_t binding, const BindSet *bind_set)
{
    assert(binding < MAX_SET);
    assert(bind_set);
    set[binding]= *bind_set;
}



}