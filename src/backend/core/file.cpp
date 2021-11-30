//
// Created by 12132 on 2021/11/21.
//

#include "file.h"
#include <cstdio>
#define STB_IMAGE_IMPLEMENTATION
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

      bool  loadHDR(const char *file, ImageInfo &imageInfo)
      {
      stbi_set_flip_vertically_on_load(true);
      imageInfo.data = stbi_loadf(file, &imageInfo.width, &imageInfo.height, &imageInfo.channels, 0);
      if(!imageInfo.data){
        return false;
      }
      return true;
    }

}