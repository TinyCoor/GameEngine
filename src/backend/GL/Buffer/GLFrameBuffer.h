//
// Created by 12132 on 2021/11/17.
//

#ifndef GAMEENGINE_GLFRAMEBUFFER_H
#define GAMEENGINE_GLFRAMEBUFFER_H
#include "GLBuffer.hpp"

class GLFrameBuffer: public GLBuffer<GL_FRAMEBUFFER> {
public:
    GLFrameBuffer(size_t width,size_t height,size_t data_size):GLBuffer<GL_FRAMEBUFFER>(width,height, data_size){

    }

    void Bind(){
        GLBuffer<GL_FRAMEBUFFER>::Bind();
    }


    void UnBind(){

    }



    //将结果显然到texture
    void BindDepthTexture(GLuint texture,int mipLevel =0){
        glNamedFramebufferTexture(this->handle,GL_DEPTH_ATTACHMENT,texture,mipLevel);
    }

    void BindColorTexture(GLuint texture,int attchment_index,int mipLevel =0){
        glNamedFramebufferTexture(this->handle,GL_COLOR_ATTACHMENT0 +attchment_index,texture,mipLevel);
    }

    //note
    void BindDepthStencilTexture(GLuint texture,int mipLevel =0){
        glNamedFramebufferTexture(this->handle,GL_DEPTH_ATTACHMENT,texture,mipLevel);
    }
};


class GLRenderBuffer : public GLBuffer<GL_RENDERBUFFER>{
public:
    GLRenderBuffer( int width,int height,int depth,GLenum format): GLBuffer<36161>(width,height,format){
        glNamedRenderbufferStorage(this->handle,width,height,format);
    }


};


#endif //GAMEENGINE_GLFRAMEBUFFER_H
