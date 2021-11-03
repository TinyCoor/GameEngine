//
// Created by y123456 on 2021/10/28.
//

#include "Texture2D.h"
int Texture2D::pos = 0;

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

bool Texture2D::loadFromFile(const std::string& file){

}

void Texture2D::copyToGPU(GLHANDLE handle){

}