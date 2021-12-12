//
// Created by 12132 on 2021/11/17.
//

#ifndef GAMEENGINE_GLSSBO_HPP
#define GAMEENGINE_GLSSBO_HPP
#include "GLBuffer.hpp"
/**
 * SSBO
 *
 * layout (std430 ) buffer light {
 *  vec3 position;
 *  vec3 ambient;
 *  vec3 diffuse;
 *  vec3 specular;
 *  };
 *
 */
class GLSSBO : public GLBuffer<GL_SHADER_STORAGE_BUFFER> {
public:
    GLSSBO(size_t width,size_t height,size_t data_size): GLBuffer<GL_SHADER_STORAGE_BUFFER>(width,height,data_size){
        this->CopyToGPU(nullptr,GL_MAP_WRITE_BIT);
    }

    void BindSSAOBuffer(int index,const GLProgram& program,const char* block_name,void* data){
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, this->handle,0,width_ * height_ * element_size_);
        glShaderStorageBlockBinding(program.GetHandle(), GetShaderStorageBlockIndex(program,block_name),index);

    }

private:
    int GetShaderStorageBlockIndex(const GLProgram& program,const char* block_name){
      return glGetProgramResourceIndex(program.GetHandle(),GL_SHADER_STORAGE_BLOCK,block_name);
    }
};


#endif //GAMEENGINE_GLSSBO_HPP
