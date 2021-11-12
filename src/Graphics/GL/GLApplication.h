//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_GLAPPLICATION_H
#define GAMEENGINE_GLAPPLICATION_H
#include <memory>
#include "ImGuiRender.h"
#include "../../Entity/camera.h"
#include <glm/glm.hpp>

class GLFWwindow;

class GLApplication{
public:
    void run();
    GLFWwindow* GetWindowHandle(){return window;}

private:

    bool init();
    void initGLFW();
    bool render();
    void shutdownGLFW() ;
    void shutdown();

private:
    GLFWwindow* window;
    std::shared_ptr<ImGuiRender> imGuiRender_{new ImGuiRender};
    // camera

};

#endif //GAMEENGINE_GLAPPLICATION_H