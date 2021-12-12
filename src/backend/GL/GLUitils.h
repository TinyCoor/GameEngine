//
// Created by 12132 on 2021/11/27.
//

#ifndef GAMEENGINE_SRC_BACKEND_GL_GLUITILS_H
#define GAMEENGINE_SRC_BACKEND_GL_GLUITILS_H
#include "glad.h"
#include "Buffer/GLBuffer.hpp"
#include "GLTexture.h"
class GLUitils {

  template<GLenum buf>
   GLBuffer<buf> createBuffer(int width, int height,size_t element_size){
     GLBuffer<buf> buffer(width,height,element_size);
      return buffer;
   }

   GLProgram createProgram(){
       GLProgram program;
       return program;
   }

   template<GLenum texture_type>
   GLTexture<texture_type> createTexture(int width,int height,GLenum format,int depth =1,int mipLevels =1){
       GLTexture<texture_type> texture(width,depth,depth,mipLevels,format);
       return texture;
   }

};

#endif //GAMEENGINE_SRC_BACKEND_GL_GLUITILS_H
