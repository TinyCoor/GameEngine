//
// Created by y123456 on 2021/10/17.
//

#include "VulkanShader.h"
#include "VulkanUtils.h"
#include <fstream>
#include <iostream>
#include <shaderc.h>
#include <cstring>

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


static shaderc_include_result* vulkan_shaderc_include_resolver(
        void* user_data,
        const char* requested_source,
        int type,
        const char* requesting_source,
        size_t include_depth)
{
//    std::cout <<"Include resolve request source: " <<requesting_source <<", target: "
//        <<requested_source <<" ,depth: " << include_depth <<std::endl;
    auto result = new shaderc_include_result();
    result->source_name= nullptr;
    result->user_data= user_data;
    result->source_name_length = 0;
    result->content = nullptr;
    result->content_length =0;

    std::string targetDir = "";

    switch (type) {
        case shaderc_include_type_standard:{
            targetDir = std::string("shaders/");
        }break;
        case shaderc_include_type_relative:{
            std::string_view sourcePath = requesting_source;
            size_t position = sourcePath.find_last_of("/\\");

            if(position !=std::string_view::npos){
                targetDir=sourcePath.substr(0,position + 1);
            }
        }
        break;
    }

    std::string targetPath = targetDir + std::string(requested_source);

    std::ifstream  file(targetPath,std::ios::ate | std::ios::binary);
    if(!file.is_open()){
        std::cerr << "VulkanShader: Laod Shader File Failed:" << "\n";
        return result;
    }
    size_t fileSize = static_cast<uint32_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(),fileSize);
    file.close();

    char * data =new char[fileSize +1];
    memset(data,0,fileSize +1);
    memcpy(data,buffer.data(),buffer.size());
    char* Path = new char[targetPath.size()+1];
    memset(Path,0,targetPath.size() +1);
    memcpy(Path,targetPath.data(),targetPath.size() + 1);

    result->source_name =Path;
    result->source_name_length= targetPath.size();
    result->content= data;
    result->content_length= fileSize;
    return result;
}


static void vulkan_shaderc_include_releaser(void* use_data,shaderc_include_result* include_result)
{
//    std::cout <<"Include result release ,Source: " << include_result->source_name <<std::endl;

    delete include_result->source_name;
    delete include_result->content;
    include_result->content= nullptr;
    include_result={0};

}




bool VulkanShader::compileFromSource(const char* path,const char* source,size_t size,shaderc_shader_kind kind)
{
 //TODO GLSL to spir byte code
    shaderc_compiler_t compiler = shaderc_compiler_initialize();
    auto options = shaderc_compile_options_initialize();

    //set compile options
    shaderc_compile_options_set_include_callbacks(options, vulkan_shaderc_include_resolver, vulkan_shaderc_include_releaser, nullptr);



    shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler,
                                                                  source,
                                                                  size,
                                                                  shaderc_glsl_infer_from_source,
                                                                  path,"main",
                                                                  options);

    if(shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success){
        std::cerr << "shderc compile error: at \"" << path << "\"" <<std::endl;
        std::cerr <<"\t";
        std::cerr << shaderc_result_get_error_message(result);
        shaderc_result_release(result);
        shaderc_compiler_release(compiler);
        return false;
    }

    auto byte_size = shaderc_result_get_length(result);
    auto * bytes = shaderc_result_get_bytes(result);

    clear();
    shaderModule = VulkanUtils::createShaderModule(context,(uint32_t*)bytes,byte_size);

    shaderc_result_release(result);
    shaderc_compiler_release(compiler);

    shader_path = path;
    return true;
}

bool VulkanShader::compileFromFile(const char* path) {
    shader_path = path;

    std::ifstream  file(path,std::ios::ate | std::ios::binary);
    if(!file.is_open()){
        std::cerr << "VulkanShader: Laod Shader File Failed:" << "\n";
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

bool VulkanShader::reload() {
    return compileFromFile(shader_path.data());
}
