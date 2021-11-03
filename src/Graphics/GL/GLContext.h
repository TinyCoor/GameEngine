//
// Created by y123456 on 2021/10/18.
//

#ifndef GAMEENGINE_GLCONTEXT_H
#define GAMEENGINE_GLCONTEXT_H
#include "GLObject.h"
//顶点属性
//TODO Special GL_DOUBLE
template<GLenum VertexType>
class VertexInputAttribute {
    int index;
    int size;
    int stride;

public:
    VertexInputAttribute(int index,size_t vertex_size,int stride)
        : index(index),
         size(vertex_size),stride(stride)
    {
    }

    ~VertexInputAttribute(){
        stride = -1;
        index = -1;
        size = 0;
    }

    void ChangeVertexInputAttribute(int index,size_t vertex_size, int stride,void* data){

    }

    void SetVertexInputAttribute(bool normalized,void* data){
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index,size,VertexType,normalized,stride,data);
    }
};

class GLContext {

};


#endif //GAMEENGINE_GLCONTEXT_H
