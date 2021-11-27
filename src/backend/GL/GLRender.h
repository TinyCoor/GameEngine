//
// Created by 12132 on 2021/11/20.
//

#ifndef GAMEENGINE_GLRENDER_H
#define GAMEENGINE_GLRENDER_H
#include "GLCubeMapRender.h"
class GLRender {
public:
    GLRender(){};
    ~GLRender(){};

    void init();

    void render();

private:
    GLCubeMapRender cubeMapRender;

};


#endif //GAMEENGINE_GLRENDER_H
