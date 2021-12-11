//
// Created by 12132 on 2021/11/13.
//

#ifndef GAMEENGINE_GLUNIFORMBUFFER_H
#define GAMEENGINE_GLUNIFORMBUFFER_H

#include "../GLObject.h"
#include "../GLProgram.h"
#include "../GLShader.h"
#include "GLBuffer.hpp"
#include <vector>

/**
 * layout(binding = 1) uniform Matrices{
 *  mat4 model;
 *  mat4 proj;
 *  mat4 view;
 * }ubo;
 *
 * BlockName Matrices
 * InstanceName ubo
 *
 */
/**
 * 推荐用法
 * 查询uniform 块的大小
 *  GetUniformSize();
 *  cpu = malloc();
 *  res = Map();
 *  memcpy(res,cpu,size)
 *  unmap()
 * 调用 map 映射到本地
 * 然后刷新到GPU
 */
struct UniformAttribute{
    const char* name;
    GLint offset;
    GLint size;
    GLint type;
    GLuint indices;
};

/**
 * Uniform Buffer 是GPU常量内存，存放的是一些常量数据
 * OpenGL context 有可以绑定很多Uniform Buffer
 * 等价于一个UniformBuffer Array 首先要绑定绑定在哪个位置
 * 即 binding index
 */
class GLUniformBuffer : public GLBuffer<GL_UNIFORM_BUFFER> {
public:
    using BindingIndex = unsigned int;
    GLUniformBuffer():GLBuffer<35345>(){}

    GLUniformBuffer(size_t size):GLBuffer<35345>(size){
        this->CopyToGPU(nullptr,size,GL_DYNAMIC_STORAGE_BIT);
    }
    GLUniformBuffer(size_t width,size_t height,GLenum format):GLBuffer<GL_UNIFORM_BUFFER>(width,height,format)
    {
        this->CopyToGPU(nullptr,width * height,GL_DYNAMIC_STORAGE_BIT);//分配GPU 缓存容量
    }

    void init(size_t size){
        this->CopyToGPU(nullptr,size,GL_DYNAMIC_STORAGE_BIT);
    }

    /**
     * 此函数使用于布局描述符layout(binding=N)没有设置
     * 将 uniform 名为name 的缓存关联到一块uniform buffer缓存中
     * eg: layout(binding = 0) uniform mat{} 那么name 就是mat
     *  binding = 0 ,name ='mat'
     * @param program
     *
     * @param name
     *          uniform block 的名称 eg:  uniform mat{} 那么name 就是mat
     * @param index
     *          绑定点
     */
    void BindUniformBuffer(int binding,const GLProgram& program,const char* block_name){
        glUniformBlockBinding(program.GetHandle(), GetUniformBlockIndex(program,block_name),binding);
        glBindBufferBase(GL_UNIFORM_BUFFER,binding,this->handle);
        this->Bind();
    }

    /**
     *  返回名为 name 的 uniform 块的绑定点
     */
    void GetUniformBindingIndex(const GLProgram& program,const char* block_name,GLint* res){
        glGetActiveUniformBlockiv(program.GetHandle(), GetUniformBlockIndex(program,block_name),GL_UNIFORM_BLOCK_BINDING,res);
    }

    /**
     *  返回名为 name 的uniform 块的大小
     */
    void GetUniformSize(const GLProgram& program,const char* block_name,GLint* res){
        glGetActiveUniformBlockiv(program.GetHandle(), GetUniformBlockIndex(program,block_name),GL_UNIFORM_BLOCK_DATA_SIZE,res);
    }

    /**
     *
     */
    void GetUniformInfomation(const GLProgram& program,const char* block_name,GLenum query,GLint* res){
        glGetActiveUniformBlockiv(program.GetHandle(), GetUniformBlockIndex(program,block_name),query,res);
    }

    void GetUniformAttribute(const GLProgram& program,std::vector<UniformAttribute>& uniforms){
        for(size_t index = 0; index < uniforms.size();++index){
            glGetUniformIndices(program.GetHandle(),1,&uniforms[index].name,&(uniforms[index].indices));
            glGetActiveUniformsiv(program.GetHandle(),1,&(uniforms[index].indices),GL_UNIFORM_OFFSET,&(uniforms[index].offset));
            glGetActiveUniformsiv(program.GetHandle(),1,&(uniforms[index].indices),GL_UNIFORM_SIZE,&(uniforms[index].size));
            glGetActiveUniformsiv(program.GetHandle(),1,&(uniforms[index].indices),GL_UNIFORM_TYPE,&(uniforms[index].type));
        }
    }

private:

    size_t TypeSize(GLenum type)
    {
        size_t size =0;
#define CASE(Enum,Count,Type) \
        case Enum:size = Count * sizeof(Type);break
        switch (type) {
            CASE(GL_FLOAT,1,GLfloat);
            CASE(GL_FLOAT_VEC2,2,GLfloat);
            CASE(GL_FLOAT_VEC3,3,GLfloat);
            CASE(GL_FLOAT_VEC4,4,GLfloat);
            CASE(GL_INT,1,GLint);
            CASE(GL_INT_VEC2,2,GLint);
            CASE(GL_INT_VEC3,3,GLint);
            CASE(GL_INT_VEC4,4,GLint);
            CASE(GL_UNSIGNED_INT,1,GLint);
            CASE(GL_UNSIGNED_INT_VEC2,2,GLuint);
            CASE(GL_UNSIGNED_INT_VEC3,3,GLuint);
            CASE(GL_UNSIGNED_INT_VEC4,4,GLuint);
            CASE(GL_BOOL,1,GLboolean);
            CASE(GL_BOOL_VEC2,2,GLboolean);
            CASE(GL_BOOL_VEC3,3,GLboolean);
            CASE(GL_BOOL_VEC4,4,GLboolean);
            CASE(GL_FLOAT_MAT2,4,GLfloat);
            CASE(GL_FLOAT_MAT2x3,6,GLfloat);
            CASE(GL_FLOAT_MAT2x4,4,GLfloat);
            CASE(GL_FLOAT_MAT3,9,GLfloat);
            CASE(GL_FLOAT_MAT3x2,6,GLfloat);
            CASE(GL_FLOAT_MAT3x4,12,GLfloat);
            CASE(GL_FLOAT_MAT4,16,GLfloat);
            CASE(GL_FLOAT_MAT4x2,8,GLfloat);
            CASE(GL_FLOAT_MAT4x3,12,GLfloat);
#undef CASE
            default:{
                assert(false && "Unreached");
            }
        }
        return size;
    }
    //返回在着色其程序的索引
    unsigned int GetUniformBlockIndex(const GLProgram& shader,const char* block_name){
        return  glGetUniformBlockIndex(shader.GetHandle(),block_name);
    }
};



inline GLUniformBuffer createUniformBuffer(size_t size){
    GLUniformBuffer uniformBuffer(size);
    return uniformBuffer;
}

#endif //GAMEENGINE_GLUNIFORMBUFFER_H
