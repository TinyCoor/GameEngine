//
// Created by y123456 on 2021/10/17.
//

#include "VulkanShader.h"
#include "VulkanUtils.h"
#include <fstream>
#include <iostream>
#include <shaderc.h>

namespace {
     shaderc_shader_kind vulkan_to_shderc_kind(ShaderKind kind) {
        switch (kind) {
            case ShaderKind::vertex: {
                return shaderc_vertex_shader;
            }
            case ShaderKind::fragment: {
                return shaderc_fragment_shader;
            }
            case ShaderKind::compute: {
                return shaderc_compute_shader;
            }
            case ShaderKind::geometry: {
                return shaderc_geometry_shader;
            }
            case ShaderKind::tessellation_control: {
                return shaderc_tess_control_shader;
            }
            case ShaderKind::tessellation_evaluation: {
                return shaderc_tess_evaluation_shader;
            }
        }
        throw std::runtime_error("not support shader kind");
    }
}

VulkanShader::~VulkanShader(){
    clear();
}

bool VulkanShader::compileFromSource(const char* path,const char* source,size_t size,shaderc_shader_kind kind)
{
 //TODO GLSL to spir byte code
    shaderc_compiler_t compiler = shaderc_compiler_initialize();
    shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler,
                                                                  source,
                                                                  size,
                                                                  shaderc_glsl_infer_from_source,
                                                                  nullptr,"main",
                                                                  nullptr);

    if(shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success){
        std::cerr << "shderc compile error: at \"" << path << "\"" <<std::endl;
        std::cerr <<"\t";
        std::cerr << shaderc_result_get_error_message(result);
        shaderc_result_release(result);
        shaderc_compiler_release(compiler);
        return false;
    }

    auto byte_size = shaderc_result_get_length(result);
    auto * bytes = reinterpret_cast<uint32_t*>(shaderc_result_get_bytes(result));

    clear();
    shaderModule = VulkanUtils::createShaderModule(context,bytes,byte_size);

    shaderc_result_release(result);
    shaderc_compiler_release(compiler);

    return true;
}

bool VulkanShader::compileFromFile(const char* path) {
    std::ifstream  file(path,std::ios::ate | std::ios::binary);
    if(!file.is_open()){
        std::cerr << "VulkanShader: Laod Shder File Failed:" << "\n";
        return  false;
    }
    clear();
    size_t fileSize = static_cast<uint32_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(),fileSize);
    file.close();

    return compileFromSource(path,buffer.data(),buffer.size(),shaderc_glsl_infer_from_source);
 
}


bool VulkanShader::compileFromFile(const char* path,ShaderKind kind) {
    std::ifstream  file(path,std::ios::ate | std::ios::binary);
    if(!file.is_open()){
        std::cerr << "VulkanShader: Laod Shder File Failed:"<< path << "\n";

        return  false;
    }
    clear();
    size_t fileSize = static_cast<uint32_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(),fileSize);
    file.close();


    return compileFromSource(path,buffer.data(),buffer.size(),shaderc_glsl_infer_from_source);
    
}

void VulkanShader::clear() {
    vkDestroyShaderModule(context.device_,shaderModule, nullptr);
    shaderModule = VK_NULL_HANDLE;
}
