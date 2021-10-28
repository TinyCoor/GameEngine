//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_GLBUFFER_HPP
#define GAMEENGINE_GLBUFFER_HPP
#include "glad.h"
#include "GLObject.h"

template<GLenum type>
class BufferPolicy{
    static constexpr GLenum buffer_type = type;
    GLHANDLE  createBufferObject(){
        GLHANDLE buffer_handle;
        glCreateBuffers(1,&buffer_handle);
        return buffer_handle;
    }

    void Bind(int pos) {

    }
};


template<GLenum buf_type, template<typename > class BufferTypePolicy = BufferPolicy>
class GLBuffer : public GLObject {
public:
    GLBuffer(): GLObject(BufferTypePolicy<buf_type>::createBufferObject(),"GLBuffer"){

    }

    void copyToGPU(){

    }

};


#endif //GAMEENGINE_GLBUFFER_HPP
