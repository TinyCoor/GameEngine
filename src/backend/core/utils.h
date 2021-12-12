//
// Created by 12132 on 2021/12/11.
//

#ifndef GAMEENGINE_SRC_APP_UTILS_H
#define GAMEENGINE_SRC_APP_UTILS_H
#include <cmath>
#include <vector>
namespace core{

const float PI =3.1415926535898;

constexpr float toRadians(float degree){
    return degree * PI / 180;
}

template <class T>
static void hash_combine(uint64_t &s, const T &v)
{
    std::hash<T> h;
    s^= h(v) + 0x9e3779b9 + (s << 6) + (s>> 2);
}


inline std::vector<float> generateSphere(float radius,float angleSpan){
    std::vector<float> vertices;
    for (int Vangle =-90; Vangle < 90;Vangle= Vangle+angleSpan) {
        for (int Hangle =0; Hangle <= 360;Hangle= Hangle + angleSpan){
            float x0= radius * cos(toRadians(Vangle)) * cos(toRadians(Hangle));
            float y0 = radius * cos(toRadians(Vangle)) * sin(toRadians(Hangle));
            float z0 = radius * sin(toRadians(Vangle));
            float x1=  radius * cos(toRadians(Vangle)) * cos(toRadians(Hangle+angleSpan));
            float y1 = radius * cos(toRadians(Vangle)) * sin(toRadians(Hangle+angleSpan));
            float z1 = radius * sin(toRadians(Vangle));
            float x2=  radius * cos(toRadians(Vangle + angleSpan)) * cos(toRadians(Hangle+angleSpan));
            float y2 = radius * cos(toRadians(Vangle + angleSpan)) * sin(toRadians(Hangle+angleSpan));
            float z2 = radius * sin(toRadians(Vangle + angleSpan));
            float x3=  radius * cos(toRadians(Vangle + angleSpan)) * cos(toRadians(Hangle));
            float y3 = radius * cos(toRadians(Vangle + angleSpan)) * sin(toRadians(Hangle));
            float z3 = radius * sin(toRadians(Vangle+angleSpan));
            vertices.push_back(x1);vertices.push_back(y1);vertices.push_back(z1);
            vertices.push_back(x3);vertices.push_back(y3);vertices.push_back(z3);
            vertices.push_back(x0);vertices.push_back(y0);vertices.push_back(z0);
        }
    }
    return vertices;
}


}
#endif //GAMEENGINE_SRC_APP_UTILS_H
