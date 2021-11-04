//
// Created by y123456 on 2021/10/28.
//

#include "Texture2D.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

int Texture2D::pos = 0;
int Texture2D::channels =0;
int Texture2D::width = 0;
int Texture2D::height = 0;
uint8_t* Texture2D:: data= nullptr;


GLHANDLE Texture2D::CreateTexture(){
    GLHANDLE  handle;
    glGenTextures(1,&handle);
    return handle;
}

std::string Texture2D::GetName(){
    return "Texture2D";
}

void Texture2D::Bind(GLHANDLE handle){
    glBindTexture(handle,GL_TEXTURE0 + pos);
    pos+=1;
}

bool Texture2D::loadFromFile(const std::string& file,GLHANDLE handle){
    uint8_t* Pixel =  stbi_load(file.c_str(),&width,&height,&channels,STBI_default);
    if(!Pixel){
        std::cerr<< "Load File Failed: "<< file << std::endl;
        exit(-1);
    }

    size_t imageSize= width * height * channels * sizeof(uint8_t);
    clearCPUData();
    data = new uint8_t[imageSize];
    memcpy(data,Pixel,imageSize);

    stbi_image_free(Pixel);
    copyToGPU(handle);

}

void Texture2D::copyToGPU(GLHANDLE handle){

}