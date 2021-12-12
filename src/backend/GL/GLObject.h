//
// Created by y123456 on 2021/10/22.
//

#ifndef GAMEENGINE_GLOBJECT_H
#define GAMEENGINE_GLOBJECT_H
#include<string>
#include "glad.h"

typedef unsigned int GLHANDLE;

// OpenGL 对象
class GLObject {
protected:
    unsigned int handle;
    std::string obj_name;
public:
    GLObject():handle(0),obj_name("GLObject"){}
    GLObject(GLHANDLE h,const std::string& objname):handle(h),obj_name(objname){}
    GLObject(const GLObject& rhs){
        handle =std::move(rhs.handle);
        obj_name= std::move(rhs.obj_name);
    }
    GLHANDLE GetHandle() const {return handle;};
    std::string GetName() const {return obj_name;};

    GLObject& operator=(GLObject& rhs){
        this->handle = rhs.handle;
        this->obj_name= rhs.obj_name;
        return *this;
    }
};

#endif //GAMEENGINE_GLOBJECT_H
