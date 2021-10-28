//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_GLPROGRAM_H
#define GAMEENGINE_GLPROGRAM_H
#include "GLObject.h"

class GLProgram : public GLObject{
public:
    GLProgram(): GLObject(glCreateProgram(),"Program"){}

    void link(){}

    //TODO select Vetex first and Fragment next
    template<typename Shader,typename ... Shaders>
    void link(Shader shader,Shaders...  otherShader){
        glAttachShader(this->handle,shader);
        link(otherShader...);
        glLinkProgram(this->handle);
    }


    void use(){
        glUseProgram(this->handle);
    }
};


#endif //GAMEENGINE_GLPROGRAM_H
