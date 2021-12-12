//
// Created by y123456 on 2021/10/18.
//

#ifndef GAMEENGINE_GLCONTEXT_H
#define GAMEENGINE_GLCONTEXT_H
#include "GLObject.h"
//顶点属性
typedef unsigned int HANDLE

enum VertexType{
    FLOAT,
    DOUBLE,
    INT,
    BYTE,
};

struct GLVertexInputAttributeDescription {

    static void SetVertexInputAttribute(int binding,int size,GLenum data_type,bool normalized,int stride,void* data){
        glVertexAttribPointer(binding,size,data_type,normalized,stride,data);
        glEnableVertexAttribArray(binding);
    }
};


struct Scissor{
    int x_ =0;
    int y_=0;
    int width_ =0;
    int height_= 0;

    Scissor(int x,int y,int width,int height):x_(x),y_(y),width_(width),height_(height){

    }

    void SetScissor(){
        glScissor(x_,y_,width_,height_);
    }
};


struct MultiSampling{

};


struct Stencil{

};



class GLContext {

};


#endif //GAMEENGINE_GLCONTEXT_H
