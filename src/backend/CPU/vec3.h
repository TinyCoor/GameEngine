//
// Created by 12132 on 2021/12/29.
//

#ifndef GAMEENGINE_SRC_BACKEND_CPU_VEC3_H
#define GAMEENGINE_SRC_BACKEND_CPU_VEC3_H
template<typename T>
class vec3 {
    vec3():{0,0,0}{}
    vec3(double x,double y,double z):{x,y,z}{}

    double x() const { return e[0];}
    double y() const { return e[1];}
    double z() const { return e[2];}

    T operator[](int i) {return e[i]}

private:
    T e[3];
};

using point3 = vec3<float>;
using vec3 = vec3<float>;


#endif//GAMEENGINE_SRC_BACKEND_CPU_VEC3_H
