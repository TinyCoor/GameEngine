//
// Created by 12132 on 2021/11/17.
//

#ifndef GAMEENGINE_GLFRAMEBUFFER_H
#define GAMEENGINE_GLFRAMEBUFFER_H
#include "GLBuffer.hpp"

/**
 * 用作离屏渲染的窗口系统,用作颜色或者深度模板附件
 * 不能用作纹理
 */
class GLRenderBuffer: public GLObject{
  int width;
  int height;
  GLenum usage;
public:
  GLRenderBuffer( int w,int h,GLenum usg =GL_DEPTH_ATTACHMENT)
    : GLObject(create(),"GLFrameBuffer") ,width(w),height(h), usage(usg){

  }
  void Bind(){
    glBindRenderbuffer(GL_RENDERBUFFER, this->handle);
  }

  void UnBind(){
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
  }

  void AllocateMemory(GLenum interformat){
    glNamedRenderbufferStorage(this->handle,interformat,width,height);
  }



private:
  GLHANDLE create() {
    GLHANDLE  handle;
    glCreateRenderbuffers(1,&handle);
    return handle;
  }

};

/**
 * 帧缓冲对象，一个帧缓冲对象有
 * COLOR_ATTACHMENT
 * DEPTH_ATTACHMENT
 * STENCIL_ATTACHMENT
 */
class GLFrameBuffer: public GLObject {
    GLenum format;
    size_t width;
    size_t height;

    GLHANDLE create(){
        GLHANDLE  handle;
        glCreateFramebuffers(1,&handle);
        return handle;
    }
public:
    GLFrameBuffer(size_t w,
                  size_t h,
                  GLenum fmt)
                  :GLObject(create(),"GLFrameBuffer") ,width(w),height(h),format(fmt)
    {}

    ~GLFrameBuffer(){
      glDeleteFramebuffers(1,&handle);
    }

    void Bind(){
      glBindFramebuffer(GL_FRAMEBUFFER, this->handle);
    }

    void UnBind(){
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    //纹理绑定到framebuffer
    void BindDepthTexture(GLuint texture,int mipLevel =0){
        glNamedFramebufferTexture(this->handle,GL_DEPTH_ATTACHMENT,texture,mipLevel);
    }

    void BindColorTexture(GLuint texture,int attchment_index,int mipLevel =0){
        glNamedFramebufferTexture(this->handle,GL_COLOR_ATTACHMENT0+ attchment_index,texture,mipLevel);
    }

    //note
    void BindDepthStencilTexture(GLuint texture,int mipLevel =0){
        glNamedFramebufferTexture(this->handle,GL_DEPTH_ATTACHMENT,texture,mipLevel);
    }

    void AttachRenderBuffer(GLRenderBuffer& rbo){
       glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,rbo.GetHandle());
    }

    bool Check(){
      auto res = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      if (res == GL_FRAMEBUFFER_COMPLETE){
        printf("frame bufffer ready");
        return true;
      }else if(res ==  GL_FRAMEBUFFER_UNDEFINED){
        printf("Undefined frame buffer\n");
        return false;
      }else if( res == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT){
        printf("not ready\n");
        return false;
      } else{
        printf("No attachment\n");
        return false;
      }
    }

    /**
    * mode
    *  GL_FRONT_LEFT,
    *  GL_FRONT_RIGHT,
    *  GL_BACK_LEFT,
    *  GL_BACK_RIGHT,
    *  GL_FRONT,
    *  GL_BACK,
    *  GL_LEFT,
    *  GL_RIGHT,
    *  GL_COLOR_ATTACHMENTi.
    *  指定从什么地方读取像素
    */

};





#endif //GAMEENGINE_GLFRAMEBUFFER_H
