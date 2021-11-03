//
// Created by y123456 on 2021/10/18.
//

#ifndef GAMEENGINE_GLTEXTURE_H
#define GAMEENGINE_GLTEXTURE_H
#include "GLObject.h"



//TODO TextureCubeMap

template<typename CreatePolicy>
class GLTexture: public GLObject{
public:
    GLTexture(): GLObject(CreatePolicy::CreateTexture(),CreatePolicy::GetName()){}


    bool loadFromFile(const std::string& path){
        CreatePolicy::loadFromFile(path);
    }

    void Bind(){
        CreatePolicy::Bind(this->handle);
    }

    void copyToGPU(){
        CreatePolicy::copyToGPU(this->handle);
    }

};


#endif //GAMEENGINE_GLTEXTURE_H
