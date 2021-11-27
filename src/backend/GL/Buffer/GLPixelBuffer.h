//
// Created by 12132 on 2021/11/17.
//

#ifndef GAMEENGINE_GLPIXELBUFFER_H
#define GAMEENGINE_GLPIXELBUFFER_H

#include "GLBuffer.hpp"
/**
 * 用于从图像中读取数据
 */
class GLReadPixelBuffer : public GLBuffer<GL_PIXEL_PACK_BUFFER>{
private:

public:
    GLReadPixelBuffer(size_t width,size_t height,GLenum pixel_type)
        :GLBuffer<GL_PIXEL_PACK_BUFFER>(width,height,pixel_type){}

    void Bind(){
        GLBuffer<GL_PIXEL_PACK_BUFFER>::Bind();
    }


};

/**
 * 作为图像的数据来源 可以作为GLTexture的图像来源
 */
class GLWritePixelBuffer : public GLBuffer<GL_PIXEL_UNPACK_BUFFER>{

public:
    GLWritePixelBuffer(size_t width,size_t height,size_t data_size)
    :GLBuffer<GL_PIXEL_UNPACK_BUFFER>(width,height,data_size)
    {}


};


#endif //GAMEENGINE_GLPIXELBUFFER_H
