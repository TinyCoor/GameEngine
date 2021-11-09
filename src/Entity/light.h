//
// Created by 12132 on 2021/11/9.
//

#ifndef GAMEENGINE_LIGHT_H
#define GAMEENGINE_LIGHT_H
#include <glm/glm.hpp>

using vec3 =glm::vec3;
using vec4 = glm::vec4;

//光照模型

//环境光 +


struct light {
    vec3 position;          // 光源位置
    vec3 color;             // 光源颜色
    float Shininess;        //高光指数
    float Strength;         //光泽强度
};


struct direction_light : public light{
    vec3 light_direction;   // 光源方向
};

struct point_light : public light{
    vec3  EyeDirection;          // 人眼位置
    float ConstantAttenuation;   //衰减系数
    float LinearAttenuation;     // 线性系数
    float QuadraticAttenuation;  // 二次项系数
};


struct spot_light : public  point_light{
    vec3 ConeDirection;     // 添加聚光灯属性
    float SpotCosCutoff;    // 聚光灯的大小，余玄值
    float SpotExponent;     // 聚光等的衰减系数
};



#endif //GAMEENGINE_LIGHT_H
