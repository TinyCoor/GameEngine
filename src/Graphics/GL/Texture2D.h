//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_TEXTURE2D_H
#define GAMEENGINE_TEXTURE2D_H
#include <stb_image.h>
#include <iostream>
#include "GLObject.h"

struct ImageInfo{
    int width;
    int height;
    int channels;
    uint8_t *data;

};
void loadTexture(const char* file,ImageInfo& imageInfo);


template<GLenum textureType>
class TextureObject{
public:
    static inline int pos =0;

    static GLHANDLE CreateTexture(){
        GLHANDLE  handle;
        glGenTextures(1,&handle);
        return handle;
    }
    /**
     * This bind GL_TEXTURE_0 and call second next is GL_TEXTURE_1
     * @param handle
     */
    static void Bind(GLHANDLE handle){
        glBindTexture(handle,GL_TEXTURE0 + pos);
        pos+=1;
    }

    static bool loadFromFile(const char* file){
        ImageInfo imageInfo;
        loadTexture(file,imageInfo);
        copyToGPU(imageInfo);
        return false;
    }

    static void copyToGPU(ImageInfo& imageInfo){

        //设置纹理属性
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageInfo.width, imageInfo.height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageInfo.data);
        glGenerateMipmap(GL_TEXTURE_2D);

        delete imageInfo.data;
    }

};




#endif //GAMEENGINE_TEXTURE2D_H
