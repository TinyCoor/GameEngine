//
// Created by 12132 on 2021/11/21.
//

#include "file.h"
#include <cstdio>
#include <stb_image.h>
#include <cstring>

namespace core{

    bool loadTexture(const char * file,ImageInfo& imageInfo){
        uint8_t* Pixel =  stbi_load(file,&imageInfo.width,&imageInfo.height,&imageInfo.channels,STBI_rgb_alpha);
        if(!Pixel){
            printf("Load File Failed: %s",file);
            return false;
        }
        size_t imageSize= imageInfo.width * imageInfo.height * imageInfo.channels * sizeof(uint8_t);
        imageInfo.data = new uint8_t[imageSize];
        memcpy(imageInfo.data,Pixel,imageSize);
        stbi_image_free(Pixel);
        return true;
    }


}