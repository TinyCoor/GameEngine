//
// Created by y123456 on 2021/10/28.
//

#include "VAO.h"

GLHANDLE CreateVeretxArrays(){
    GLHANDLE  handle=0;
    glCreateVertexArrays(1,&handle);
    return handle;
}

GLHANDLE CreateVertexBufferObject(){
    GLHANDLE handle;
    glGenBuffers(1,&handle);
    return handle;
}



