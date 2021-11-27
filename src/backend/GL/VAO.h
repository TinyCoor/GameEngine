//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_VAO_H
#define GAMEENGINE_VAO_H
#include "GLObject.h"
#include <vector>
#include "glad.h"

struct VertexInputBindings{
    int position;
    GLenum data_type;
};



/**
 * 顶点数组对象
 */
class Vao : public GLObject{
public:
    Vao(): GLObject(createVAO(),std::string("VAO")){};

    void Bind(){
        glBindVertexArray(this->handle);
    }

    void UnBind(){
        glBindVertexArray(0);
    }

private:
    GLHANDLE createVAO(){
        GLHANDLE  glhandle = 0;
        glCreateVertexArrays(1,&glhandle);
        return glhandle;
    }
};



#endif //GAMEENGINE_VAO_H
