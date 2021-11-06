//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_TEXTURECUBEMAP_H
#define GAMEENGINE_TEXTURECUBEMAP_H
#include "GLObject.h"
#include "GLTexture.h"
/**
 * This is Used to Cubemap 立方体贴图
 * 简称 skybox
 */

void loadCubeTexture(const char* file,ImageInfo& imageInfo);

//TODO
 void loadHDRTexture(const char* file,ImageInfo& info);


template<>
class TextureObject<GL_TEXTURE_CUBE_MAP> {
    static GLHANDLE CreateTexture(){
        unsigned int textureID;
        glGenTextures(1,&textureID);
        return textureID;
    }

    /**
     * This bind GL_TEXTURE_0 and call second next is GL_TEXTURE_1
     * @param handle
     */

    static void Bind(GLHANDLE handle){
        glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
    }

    static bool loadFromFile(const std::string& file){
        return true;
    }
    static void copyToGPU(GLHANDLE handle)
    {

    }
};


#endif //GAMEENGINE_TEXTURECUBEMAP_H
