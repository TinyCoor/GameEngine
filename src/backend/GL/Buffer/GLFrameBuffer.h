//
// Created by 12132 on 2021/11/17.
//

#ifndef GAMEENGINE_GLFRAMEBUFFER_H
#define GAMEENGINE_GLFRAMEBUFFER_H
#include "GLBuffer.hpp"
#include "../GLTexture.h"

enum class attachment_type{
    color_attachment = 0,
    depth_attachment,
    depth_stencil_attachment,
};

struct attachment{
    attachment_type type;
    GLTexture<GL_TEXTURE_2D> texture;
};

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
/**
  * MRT 将场景渲染多个目标中
  * fragShader
  * layout(location = 0) out vec4 outColor0;
  * layout(location = 1) out vec4 outColor1;
  * layout(location = 2) out vec4 outColor2;
  * layout(location = 3) out vec4 outColor3;
  * layout(location = 4) out vec4 outColor4;
  * layout(location = 5) out vec4 outColor5;
  *
  * c++
  *     GLenum Buffers[6];
  *     for(int i=0; i< 6;++i){
  *         glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0+i,GL_TEXTURE_2D,textures[i]);
  *     }
  *     Buffers[0] = GL_COLOR_ATTACHMENT0;
  *     Buffers[1] = GL_COLOR_ATTACHMENT1;
  *     Buffers[2] = GL_COLOR_ATTACHMENT2;
  *     Buffers[3] = GL_COLOR_ATTACHMENT3;
  *     Buffers[4] = GL_COLOR_ATTACHMENT4;
  *     Buffers[5] = GL_COLOR_ATTACHMENT5;
  *     glDrawBuffers(6,Buffers);
  */

class GLFrameBuffer: public GLObject {
    GLenum format;
    size_t width;
    size_t height;

    GLHANDLE create() {
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

    ~GLFrameBuffer() {
      glDeleteFramebuffers(1,&handle);
    }

    void Bind() {
      glBindFramebuffer(GL_FRAMEBUFFER, this->handle);
    }

    void UnBind() {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    //纹理绑定到framebuffer
    template<GLenum textureType>
    void BindDepthTexture(GLTexture<textureType>& texture) {
        glNamedFramebufferTexture(this->handle,GL_DEPTH_ATTACHMENT,texture,texture.getMipLevel());
    }

    void BindColorTexture(GLuint texture,int attchment_index,int mipLevel =0) {
        glNamedFramebufferTexture(this->handle,GL_COLOR_ATTACHMENT0 + attchment_index,texture,mipLevel);
    }

    //note
    void BindDepthStencilTexture(GLuint texture,int mipLevel =0) {
        glNamedFramebufferTexture(this->handle,GL_DEPTH_ATTACHMENT,texture,mipLevel);
    }

    ///
    void AttachRenderBuffer(GLRenderBuffer& rbo) {
       glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,rbo.GetHandle());
    }

    bool Check() {
      auto res = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      if (res == GL_FRAMEBUFFER_COMPLETE){
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
