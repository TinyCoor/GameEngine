//
// Created by 12132 on 2021/11/7.
//

#ifndef GAMEENGINE_TEXTURETRAITS_H
#define GAMEENGINE_TEXTURETRAITS_H
#include "GLObject.h"
template<GLenum textureType>
struct texture_trait{
    static constexpr int dim = -1;
    static constexpr bool supportMipmap = false;
};


template<>
struct texture_trait<GL_TEXTURE_BUFFER> {
    static constexpr int dim = 1;
    static constexpr bool supportMipmap = true;
};

template<>
struct texture_trait<GL_TEXTURE_1D> {
    static constexpr int dim = 1;
    static constexpr bool supportMipmap = true;
};

template<>
struct texture_trait<GL_TEXTURE_1D_ARRAY> {
    static constexpr int dim = 1;
    static constexpr bool supportMipmap = true;
};

template<>
struct texture_trait<GL_TEXTURE_2D> {
    static constexpr int dim = 2;
    static constexpr bool supportMipmap = true;
};

template<>
struct texture_trait<GL_TEXTURE_CUBE_MAP> {
    static constexpr int dim = 2;
    static constexpr bool supportMipmap = true;
};

template<>
struct texture_trait<GL_TEXTURE_2D_MULTISAMPLE> {
    static constexpr int dim = 2;
    static constexpr bool supportMipmap = true;
};


template<>
struct texture_trait<GL_TEXTURE_2D_ARRAY> {
    static constexpr int dim = 3;
    static constexpr bool supportMipmap = true;
};

template<>
struct texture_trait<GL_TEXTURE_2D_MULTISAMPLE_ARRAY> {
    static constexpr int dim = 3;
    static constexpr bool supportMipmap = true;
};

template<>
struct texture_trait<GL_TEXTURE_CUBE_MAP_ARRAY> {
    static constexpr int dim = 3;
    static constexpr bool supportMipmap = true;
};

template<>
struct texture_trait<GL_TEXTURE_RECTANGLE> {
    static constexpr int dim = 2;
    static constexpr bool supportMipmap = false;
};


template<>
struct texture_trait<GL_TEXTURE_3D> {
    static constexpr int dim = 3;
    static constexpr bool supportMipmap = true;
};










#endif //GAMEENGINE_TEXTURETRAITS_H
