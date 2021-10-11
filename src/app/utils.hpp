//
// Created by y123456 on 2021/10/10.
//

#ifndef GAMEENGINE_UTILS_HPP
#define GAMEENGINE_UTILS_HPP

#include <string>
#include <fstream>
#include <vector>

static std::vector<char> readFile(const std::string& filename){
    std::ifstream file(filename,std::ios::ate | std::ios::binary);
    if(!file.is_open()){
        throw std::runtime_error("open file failed");
    }

    size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(),file_size);
    file.close();
    return buffer;
}

#endif //GAMEENGINE_UTILS_HPP
