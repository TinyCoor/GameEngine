//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_GLAPPLICATION_H
#define GAMEENGINE_GLAPPLICATION_H


class GLFWwindow;
class GLApplication{
public:
    void run();
private:
    void initGLFW();
    void initImGui();

    void shutdownImGui();
    bool render();

    void shutdownGLFW();

private:
    GLFWwindow* window;
};

#endif //GAMEENGINE_GLAPPLICATION_H