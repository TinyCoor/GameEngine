//
// Created by y123456 on 2021/10/18.
//

#ifndef GAMEENGINE_GLTEXTURE_H
#define GAMEENGINE_GLTEXTURE_H
#include "GLObject.h"
#include "Texture2D.h"


//TODO TextureCubeMap



template<GLenum textureType, template<GLenum > class TextureCreatePolicy=Texture2D>
class GLTexture: public GLObject{
public:
    GLTexture(): GLObject(TextureCreatePolicy<textureType>::CreateTexture(),"Texture"){

    }


    bool loadFromFile(const std::string& path){
       return TextureCreatePolicy<textureType>::loadFromFile(path.c_str());
    }

    void Bind(){
        TextureCreatePolicy<textureType>::Bind(this->handle);
    }

//    void copyToGPU(){
//        TextureCreatePolicy<textureType>::copyToGPU(this->handle);
//    }

};


#endif //GAMEENGINE_GLTEXTURE_H
