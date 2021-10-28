//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_VAO_H
#define GAMEENGINE_VAO_H
#include "GLObject.h"
#include <vector>


struct VertexInputBindings{
    int position;
    GLenum data_type;
};

class EBO :public GLObject{
public:
    EBO();


};

/**
 * 顶点数组缓存对象
 */
class VBO: public GLObject{
public:
    VBO();

    //激活当前对象,通知OpenGL 正在使用当前对象，操作对象
    void Bind();

    //分配 size 大小 GPU 缓存
    void AllocateGPUBuffer(int size, GLbitfield flags){
        glNamedBufferData(this->handle,size, nullptr,flags);
    }

    //将以data开始的数据发送给GPU端
    void copyToGPU(int size, const void* data,GLbitfield flags){
        glNamedBufferData(this->handle,size, data,flags);
    }

private:


};


/**
 * 顶点数组对象
 */
class VAO : public GLObject{
public:
    VAO();

    void Bind();

private:

};




#endif //GAMEENGINE_VAO_H
