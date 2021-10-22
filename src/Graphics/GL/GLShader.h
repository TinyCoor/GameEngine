//
// Created by y123456 on 2021/10/18.
//

#ifndef GAMEENGINE_GLSHADER_H
#define GAMEENGINE_GLSHADER_H
#include<string>

enum class ShaderKind{
    vertex =0,
    fragment,
    compute,
    geometry,
    tessellation_control,
    tessellation_evaluation,
};


class GLShader {
public:
    GLShader();

    bool loadFromFile(const std::string,ShaderKind type);

private:
    unsigned int handle;
};


#endif //GAMEENGINE_GLSHADER_H
