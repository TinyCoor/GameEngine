//
// Created by y123456 on 2021/10/22.
//

#ifndef GAMEENGINE_GLOBJECT_H
#define GAMEENGINE_GLOBJECT_H
#include<string>
#include "glad.h"

enum ObjectType{
    VAO,
    VBO,
    EBO,
    shader,
    texture,
    buffer,
};

typedef unsigned int GLHANDLE;

class GLObject {
protected:
    unsigned int handle;
    std::string obj_name;
public:
    GLObject(GLHANDLE h,const std::string& objname):handle(h),obj_name(objname){}
    GLHANDLE GetHandle() {return handle;};
    std::string GetName() {return obj_name;};
   // virtual ObjectType GetType() = 0;
};

#endif //GAMEENGINE_GLOBJECT_H
