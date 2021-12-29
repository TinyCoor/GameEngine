//
// Created by 12132 on 2021/12/29.
//

#ifndef GAMEENGINE_SRC_BACKEND_CPU_RAY_H
#define GAMEENGINE_SRC_BACKEND_CPU_RAY_H
#include "vec3.h"
class ray {
public:
    ray();
    ray(point3 ori,vec3 dir):origin(ori),direction(dir){}

    point3 at(float t) const {return origin + t * direction}

private:
    point3 origin;
    vec3 direction;
};
#endif//GAMEENGINE_SRC_BACKEND_CPU_RAY_H
