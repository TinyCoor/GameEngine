//
// Created by 12132 on 2021/11/4.
//

#ifndef GAMEENGINE_GLQUERY_H
#define GAMEENGINE_GLQUERY_H
#include "GLObject.h"


/**
 * 遮挡查询
 * @tparam target
 *  GL_SAMPLES_PASSED,
 *  GL_ANY_SAMPLES_PASSED,
 *  GL_ANY_SAMPLES_PASSED_CONSERVATIVE,
 *  GL_TIME_ELAPSED, GL_TIMESTAMP,
 *  GL_PRIMITIVES_GENERATED or GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN.
 */

template<GLenum target>
class GLQuery : public GLObject{
public:
    GLQuery() : GLObject(createQuery(),"Query"){}
    ~GLQuery(){

    };

    void BeginQuery(){
        glBeginQuery(target,this->handle);
    }

    void EndQuery(){
        glEndQuery(target);
    }

    bool GetQueryResult(GLenum pname,GLint* params){
        //glGetQueryBufferObjectiv(this->handle,pname,params);
        return false;
    }

    void DestroyQuery(){
        glDeleteQueries(1, &this->handle);
    }

private:
    GLHANDLE  createQuery(){
        GLHANDLE handle;
        glCreateQueries(target,1,&handle);
        return handle;
    }
};


#endif //GAMEENGINE_GLQUERY_H
