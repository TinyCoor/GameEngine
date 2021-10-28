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

    }

    bool  readFile(const std::string& file,std::vector<char>& buffer){
        std::ifstream input(file,std::ios::ate | std::ios::binary);
        if(!input.is_open()){
            std::cerr << "Read File Error: " << file <<std::endl;
            return false;
        }
        size_t file_size = static_cast<size_t>(input.tellg());
        buffer.resize(file_size);
        input.seekg(0);
        input.read(buffer.data(),(long long)file_size);
        input.close();
        return true;
    }

}


GLShader::GLShader(ShaderKind type):
GLObject(glCreateShader(ToGLShaderType(type)),std::string( "VertexShader")){
    this->type =type;
}

bool GLShader::compileFromFile(const std::string& file_name) {
    std::vector<char> buffer;
    const int length = buffer.size();
    const char* data = buffer.data();
    if(readFile(file_name,buffer)){
        glShaderSource(this->handle, 1, reinterpret_cast<const GLchar *const *>(data), &length );
        glCompileShader(this->handle);
        return GetCompileError();
    }
    return false;

}

 bool GLShader::GetCompileError() {
    int result;
    glGetShaderiv(this->handle,GL_COMPILE_STATUS,&result);
    char log[1024]={0};
    if(result == GL_FALSE){
        glGetShaderInfoLog(this->handle,1024,&result,log);
        std::cerr << log;
        return false;
    }
     return true;
}
