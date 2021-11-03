//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_GLPROGRAM_H
#define GAMEENGINE_GLPROGRAM_H
#include "GLObject.h"



class GLProgram : public GLObject{

public:
    friend class GLUniformBlock;
    GLProgram(): GLObject(glCreateProgram(),"Program"){}

    ~GLProgram(){
        glDeleteProgram(this->handle);
    }

    void link(){}

    //TODO select Vetex first and Fragment next
    template<typename Shader,typename ... Shaders>
    void link(Shader shader,Shaders...  otherShader){
        glAttachShader(this->handle,shader);
        link(otherShader...);
        glLinkProgram(this->handle);
    }

    void use(){
        glUseProgram(this->handle);
    }

    void SetUniformInt(const char* name,int value){
        glUniform1d(GetUniformLocation(name),value);
    }

    void SetUniformMatrix3f(const char* name,GLsizei count, GLboolean transpose,GLfloat* values){
        glUniformMatrix3fv(GetUniformLocation(name),count,transpose,values);
    }

    void SetUniformFloat(const char* name,float value){
        glUniform1f(GetUniformLocation(name),value);
    }

    void SetUniformVec3f(const char* name,float f1,float f2,float f3){
        glUniform3f(GetUniformLocation(name),f1,f2,f3);
    }
    void SetUniformVec4f(const char* name,float f1,float f2,float f3,float f4){
        glUniform4f(GetUniformLocation(name),f1,f2,f3,f4);
    }



private:
    //返回name所对应的uniform 变量的索引值
    GLint  GetUniformLocation(const char* name){
        return glGetUniformLocation(this->handle,name);
    }

    //TODO impl Error information
    void GetErrorInformation(){

    }


};

struct GLUniformBlock: public GLObject{

   void  GetUniformInformation(const GLProgram& program,const char* UniformBlockName,int size){
        glGetActiveUniformBlockiv(program.handle, GetUniformBlockIndex(program,UniformBlockName),
                                  GL_UNIFORM_BLOCK_DATA_SIZE,&size);
    }

private:

    GLint GetUniformBlockIndex(const GLProgram& program,const char* UniformBlockName){
        return glGetUniformBlockIndex(program.handle,UniformBlockName);
    }
    void* Buffer;
};


#endif //GAMEENGINE_GLPROGRAM_H
