//
// Created by y123456 on 2021/10/22.
//

#ifndef GAMEENGINE_GL_H
#define GAMEENGINE_GL_H

#include "glcorearb.h"

using PFNGETGLPROC = void* (const char*);

struct GL4API
{
#include "GLAPI.h"
};

void GetAPI4(GL4API* api, PFNGETGLPROC GetGLProc);
void InjectAPITracer4(GL4API* api);

#endif //GAMEENGINE_GL_H
