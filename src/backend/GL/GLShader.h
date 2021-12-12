//
// Created by y123456 on 2021/10/18.
//

#ifndef GAMEENGINE_GLSHADER_H
#define GAMEENGINE_GLSHADER_H
#include <string>
#include "GLObject.h"

enum class ShaderKind{
    vertex =0,
    fragment,
    compute,
    geometry,
    tessellation_control,
    tessellation_evaluation,
};

//TODO support spv format
class GLShader : public  GLObject{
public:

    GLShader(ShaderKind type);
    ~GLShader();

    void init() {

    }

    bool compileFromFile(const std::string& path);


    inline ShaderKind GetShaderType(){ return type;}

private:
     bool GetCompileError(const std::string& file);

private:
    ShaderKind type;
};


class SpirShader: public GLObject{
public:
    SpirShader(ShaderKind type);
    ~SpirShader();

    bool loadSpirCode(const char *spir_path,const char* entry_point = "main");

private:

};






#endif //GAMEENGINE_GLSHADER_H
