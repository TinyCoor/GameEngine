//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_TEXTURE2D_H
#define GAMEENGINE_TEXTURE2D_H
#include "GLObject.h"

class Texture2D{
    static int pos;
    static GLHANDLE CreateTexture();
    static std::string GetName();
    /**
     * This bind GL_TEXTURE_0 and call second next is GL_TEXTURE_1
     * @param handle
     */
    static void Bind(GLHANDLE handle);
    static bool loadFromFile(const std::string& file);
    static void copyToGPU(GLHANDLE handle);
};

#endif //GAMEENGINE_TEXTURE2D_H
