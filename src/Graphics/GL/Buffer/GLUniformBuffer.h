//
// Created by 12132 on 2021/11/13.
//

#ifndef GAMEENGINE_UNIFORMBLOCK_H
#define GAMEENGINE_UNIFORMBLOCK_H

#include "GLObject.h"
#include "GLProgram.h"
#include "GLShader.h"
#include "GLBuffer.hpp"
#include <vector>

/**
 * layout(binding = 1) uniform Matrices{
 *  mat4 model;
 *  mat4 proj;
 *  mat4 view;
 * }ubo;
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


class GLUniformBlock : public GLBuffer<GL_UNIFORM_BUFFER> {
public:
    using BindingIndex = unsigned int;
    GLUniformBlock():GLBuffer<GL_UNIFORM_BUFFER>(){}

    /**
     * 将 uniform 名为name 的缓存关联到一块uniform buffer缓存中
     * eg: layout(binding = 0) uniform mat{} 那么name 就是mat
     *  index 为 1
     * @param program
     *
     * @param name
     *          uniform block 的名称 eg:  uniform mat{} 那么name 就是mat
     * @param index
     *          绑定点
     */
    void BindUniformBuffer(const GLProgram& program,const char* name,void* data,uint32_t size){
        this->CopyToGPU(data,size,GL_MAP_WRITE_BIT);
        glBindBufferBase(GL_UNIFORM_BUFFER,GetUniformBindingLoc(program,name),this->handle);
    }

    /**
     *  返回名为 name 的uniform 块的绑定点
     */
    void GetUniformBindingIndex(const GLProgram& program,const char* name,GLint* res){
        glGetActiveUniformBlockiv(program.handle, GetUniformBindingLoc(program,name),GL_UNIFORM_BLOCK_BINDING,res);
    }

    /**
     *  返回名为 name 的uniform 块的大小
     */
    void GetUniformSize(const GLProgram& program,const char* name,GLint* res){
        glGetActiveUniformBlockiv(program.handle, GetUniformBindingLoc(program,name),GL_UNIFORM_BLOCK_DATA_SIZE,res);
    }

    /**
     *
     */
    void GetUniformInfomation(const GLProgram& program,const char* name,GLenum query,GLint* res){
        glGetActiveUniformBlockiv(program.handle, GetUniformBindingLoc(program,name),query,res);
    }


    void GetUniformAttribute(const GLProgram& program,std::vector<UniformAttribute>& uniforms){
        for(size_t index = 0; index < uniforms.size();++index){
            glGetUniformIndices(program.handle,1,&uniforms[index].name,&(uniforms[index].indices));
            glGetActiveUniformsiv(program.handle,1,&(uniforms[index].indices),GL_UNIFORM_OFFSET,&(uniforms[index].offset));
            glGetActiveUniformsiv(program.handle,1,&(uniforms[index].indices),GL_UNIFORM_SIZE,&(uniforms[index].size));
            glGetActiveUniformsiv(program.handle,1,&(uniforms[index].indices),GL_UNIFORM_TYPE,&(uniforms[index].type));
        }
    }



private:
    //返回在着色其程序的索引
    BindingIndex GetUniformBindingLoc(const GLProgram& shader,const char* name){
        return  glGetUniformBlockIndex(shader.handle,name);
    }
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

};


#endif //GAMEENGINE_UNIFORMBLOCK_H
