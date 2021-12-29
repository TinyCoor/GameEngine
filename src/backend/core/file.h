//
// Created by 12132 on 2021/11/21.
//

#ifndef GAMEENGINE_FILE_H
#define GAMEENGINE_FILE_H

typedef unsigned char uint8_t;
namespace core {
    struct ImageInfo{
        int width = 0;
        int height =0;
        int channels = 0;
        void *data = nullptr;
    };
    bool loadTexture(const char* file,ImageInfo& imageInfo);

    bool loadHDR(const char* file,ImageInfo& imageInfo);
}

#endif //GAMEENGINE_FILE_H
