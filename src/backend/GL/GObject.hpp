//
// Created by 12132 on 2021/11/4.
//

#ifndef GAMEENGINE_GOBJECT_HPP
#define GAMEENGINE_GOBJECT_HPP

#endif //GAMEENGINE_GOBJECT_HPP
#include "glad.h"

template<GLenum type,typename Handle>
class DefaultObjectCreateAndDestroy{
public:
    static Handle CreateObject(){
        Handle handle;
        glCreateBuffers(1,&handle);
        return handle;
    }

    static void Bind(Handle handle){
        glBindBuffer(type,handle);
    }

    static void Destroy(Handle handle){
        glDeleteBuffers(1,handle);
    }

};


template<typename Handle>
class DefaultObjectCreateAndDestroy<GL_VERTEX_ARRAY,Handle>{
public:
    static Handle CreateObject(){
        Handle handle;
        glGenVertexArrays(1,&handle);
        return handle;
    }

    static void Bind(Handle handle){
        glBindVertexArray(handle);
    }

    static void Destroy(Handle handle){
        glBindVertexArray(0);
    }
};


template<typename Handle>
class DefaultObjectCreateAndDestroy<GL_SHADER,Handle>{
public:
    static Handle CreateObject(){
        Handle handle;
        glCreateShader(1,&handle);
        return handle;
    }

    static void Bind(){

    }


    static void Destroy(Handle handle){
        glDeleteShader(handle);
    }
};

template<typename Handle>
class DefaultObjectCreateAndDestroy<GL_PROGRAM,Handle>{
public:
    static Handle CreateObject(){
        Handle handle;
        glCreateProgram(1,&handle);
        return handle;
    }

    static void Bind(Handle handle){
        glUseProgram(handle);
    }

    static void Destroy(Handle handle){
        glDeleteProgram(handle);
    }
};




typedef unsigned int GLHANDLE;
template<GLenum GLType, typename Handle,template<GLenum,typename > class ObjectCreateDestroy>
class GObject{
    GLHANDLE handle;
    GObject():handle(ObjectCreateDestroy<GLType,Handle>::CreateObject())
    {}

    void Bind(){
        ObjectCreateDestroy<GLType,Handle>::CreateObject();
    }


    void Destroy(){
        ObjectCreateDestroy<GLType,Handle>::Destroy(this->handle);
    }


};