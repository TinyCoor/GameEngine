//
// Created by 12132 on 2021/11/9.
//

#ifndef GAMEENGINE_MATERIAL_H
#define GAMEENGINE_MATERIAL_H
#include <glm/glm.hpp>

using vec3 =glm::vec3;


struct material_properties{
    vec3 emission;  //材质的发光度
    vec3 ambient;  //反射环境光的部分
    vec3 diffuse;  //漫反射部分
    vec3 specular  //高光
    float shininess //高光指数
};


#endif //GAMEENGINE_MATERIAL_H
