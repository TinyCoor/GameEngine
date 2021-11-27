//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_GLPROGRAM_H
#define GAMEENGINE_GLPROGRAM_H
#include "GLObject.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

//todo support spv format
class GLProgram : public GLObject{

public:
    GLProgram(): GLObject(glCreateProgram(),"Program"){}

    ~GLProgram(){
        glDeleteProgram(this->handle);
    }

    void init(){
        this->handle = glCreateProgram();
    }

    template<typename ... Shaders>
    void link(){

    }

    //TODO select Vetex first and Fragment next
    template<typename Shader,typename ... Shaders>
    void link(Shader shader,Shaders...  otherShader){
        glAttachShader(this->handle,shader.GetHandle());
        link(otherShader...);
        glLinkProgram(this->handle);
        GetErrorInformation();
    }


    /**
     *
     * @param programInterface
     *
     * @param name
     *
     * @return
     */
   unsigned int ProgramResourceIndex(GLenum programInterface,const char * name)
   {
      return glGetProgramResourceIndex(this->handle,programInterface,name);
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

    void SetUniformMatrix4fv(const char* name, int count, GLboolean transpose, glm::mat4& matrix)
    {
        glUniformMatrix4fv(GetUniformLocation(name),count,transpose,glm::value_ptr(matrix));
    }

    void SetUniformFloat(const char* name,float value){
        glUniform1f(GetUniformLocation(name),value);
    }

    void SetUniformVec3f(const char* name,float f1,float f2,float f3){
        glUniform3f(GetUniformLocation(name),f1,f2,f3);
    }
    void SetUniformVec3f(const char* name,glm::vec3& value){
        glUniform3f(GetUniformLocation(name),value.x,value.y,value.z);
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
        int result;
        glGetProgramiv(this->handle,GL_LINK_STATUS,&result);
        char log[1024]={0};
        if(result == GL_FALSE){
            glGetProgramInfoLog(this->handle,1024,&result,log);
            std::cerr <<"Program" <<"\n" << log;
            exit(-1);
        }
    }


};



inline GLProgram createProgram(){
    GLProgram program;
    program.init();
    return program;
}



#endif //GAMEENGINE_GLPROGRAM_H