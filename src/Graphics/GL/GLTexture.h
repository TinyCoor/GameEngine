//
// Created by y123456 on 2021/10/18.
//

#ifndef GAMEENGINE_GLTEXTURE_H
#define GAMEENGINE_GLTEXTURE_H
#include "GLObject.h"

class Texture2D{
    static int pos;
    static GLHANDLE CreateTexture(){
        GLHANDLE  handle;
        glGenTextures(1,&handle);
        return handle;
    }

    static std::string GetName(){
        return "Texture2D";
    }

    static void Bind(GLHANDLE handle){
        glBindTexture(handle,GL_TEXTURE0 + pos);
    }

    static void copyToGPU(GLHANDLE handle){

    }

};

template<typename CreatePolicy>
class GLTexture: public GLObject{
public:
    GLTexture(): GLObject(CreatePolicy::CreateTexture(),CreatePolicy::GetName()){}

    void Bind(){
        CreatePolicy::Bind(this->handle);
    }

    void copyToGPU(){
        CreatePolicy::copyToGPU(this->handle);
    }

};


#endif //GAMEENGINE_GLTEXTURE_H
