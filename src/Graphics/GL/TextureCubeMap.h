//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_TEXTURECUBEMAP_H
#define GAMEENGINE_TEXTURECUBEMAP_H

/**
 * This is Used to Cubemap 立方体贴图
 * 简称 skybox
 */
class TextureCubeMap {
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


#endif //GAMEENGINE_TEXTURECUBEMAP_H
