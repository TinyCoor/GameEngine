//
// Created by 12132 on 2021/12/8.
//

#ifndef GAMEENGINE_SRC_BACKEND_GL_GLPIPELINE_H
#define GAMEENGINE_SRC_BACKEND_GL_GLPIPELINE_H
#include "GLObject.h"

template<GLenum shaderType>
class AdvanceShaderCreation
{
public:
    static GLHANDLE create(const char *file_content)
    {
        GLHANDLE handle = 0;
        handle = glCreateShaderProgramv(shaderType, 1, &file_content);
        return handle;
    }
};

template<GLenum shaderType, template<GLenum> class ShaderCreate =AdvanceShaderCreation>
class GLProgramStage : public GLObject
{
public:
    GLProgramStage(const char *shaderSrc) : GLObject(ShaderCreate<shaderType>::create(shaderSrc), "shaderStages")
    {}
    ~GLProgramStage()
    {}
};

class GLPipeline : GLObject
{
public:
    GLPipeline() : GLObject(create(), "pipeline")
    {}

    ~GLPipeline()
    {
        glDeleteProgramPipelines(1, &handle);
    }

    template<GLenum shaderStage>
    void attachProgramStages(GLProgramStage<shaderStage> &stage)
    {
        glUseProgramStages(handle, shaderStage, stage.GetHandle());
    }

    void Bind()
    {
        glBindProgramPipeline(handle);
    }

private:
    GLHANDLE create()
    {
        GLHANDLE pipeline = 0;
        glCreateProgramPipelines(1, &pipeline);
        return pipeline;
    }

};

#endif //GAMEENGINE_SRC_BACKEND_GL_GLPIPELINE_H
