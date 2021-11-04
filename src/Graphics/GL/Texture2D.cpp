//
// Created by y123456 on 2021/10/28.
//

#include "Texture2D.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

void loadTexture(const char * file,ImageInfo& imageInfo){
    uint8_t* Pixel =  stbi_load(file,&imageInfo.width,&imageInfo.height,&imageInfo.channels,0);
    if(!Pixel){
        std::cerr<< "Load File Failed: "<< file << std::endl;
        exit(-1);
    }
    size_t imageSize= imageInfo.width * imageInfo.height * imageInfo.channels * sizeof(uint8_t);
    imageInfo.data = new uint8_t[imageSize];
    memcpy(imageInfo.data,Pixel,imageSize);
    stbi_image_free(Pixel);
}


