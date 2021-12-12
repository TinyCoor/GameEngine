//
// Created by y123456 on 2021/10/18.
//

#include "GLShader.h"
#include <cassert>
#include <vector>
#include <fstream>
#include <iostream>

namespace {
    GLenum ToGLShaderType(ShaderKind kind){
        switch (kind) {
            case ShaderKind::vertex: return GL_VERTEX_SHADER;
            case ShaderKind::fragment: return GL_FRAGMENT_SHADER;
            case ShaderKind::tessellation_control: return GL_TESS_CONTROL_SHADER ;
            case ShaderKind::tessellation_evaluation: return GL_TESS_EVALUATION_SHADER;
            case ShaderKind::compute: return GL_COMPUTE_SHADER;
            case ShaderKind::geometry: return GL_GEOMETRY_SHADER;

            default:{
                assert(false && "Unreachable Code");
            }

        }
        return 0;

    }

    bool  readFile(const std::string& file,std::vector<uint8_t>& buffer){
        std::ifstream input(file,std::ios::ate | std::ios::binary);
        if(!input.is_open()){
            std::cerr << "Read File Error: " << file <<std::endl;
            return false;
        }
        size_t file_size = static_cast<size_t>(input.tellg());
        buffer.resize(file_size);
        input.seekg(0);
        char * data_begin = reinterpret_cast<char *>(buffer.data());
        input.read(data_begin,(long long)file_size);
        input.close();
        return true;
    }

}


GLShader::GLShader(ShaderKind type):
GLObject(glCreateShader(ToGLShaderType(type)),std::string( "VertexShader")){
    this->type =type;
}

bool GLShader::compileFromFile(const std::string& file_name) {
    std::vector<uint8_t> buffer;

    if(readFile(file_name,buffer)){
        const int length = buffer.size();
        const GLchar* data[] = {(char*)( buffer.data())};
        glShaderSource(this->handle, 1, data, &length );
        glCompileShader(this->handle);
        return GetCompileError(file_name);
    }
    return false;

}

 bool GLShader::GetCompileError(const std::string& file) {
    int result;
    glGetShaderiv(this->handle,GL_COMPILE_STATUS,&result);
    char log[1024]={0};
    if(result == GL_FALSE){
        glGetShaderInfoLog(this->handle,1024,&result,log);
        std::cerr <<file <<"\n" << log;
        return false;
    }
     return true;
}

