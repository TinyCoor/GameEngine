//
// Created by 12132 on 2021/11/12.
//

#ifndef GAMEENGINE_EVENT_H
#define GAMEENGINE_EVENT_H
#include <functional>
class GLFWwindow;

class GLFWEvent{
public:
    using cb = std::function<void(GLFWwindow*,double ,double )>;
    GLFWEvent() =default;


};


#endif //GAMEENGINE_EVENT_H
