//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_GLAPPLICATION_H
#define GAMEENGINE_GLAPPLICATION_H
#include <memory>
#include "ImGuiRender.h"
#include "../../Entity/camera.h"
#include "GLRender.h"
#include <glm/glm.hpp>

class GLFWwindow;


struct CameraState
{
    double phi {0.0f};
    double theta {0.0f};
    double radius {2.0f};
    glm::vec3 target;
};

struct InputState
{
    bool firstMouse = true;
    const double rotationSpeed {0.01};
    const double scrollSpeed {1.5};
    bool rotating {false};
    double lastMouseX {0.0};
    double lastMouseY {0.0};
};


class GLApplication{
public:
    GLApplication();
    void run();
    GLFWwindow* GetWindowHandle(){return window;}


    //GLFW Event
    static void OnFrameBufferResized(GLFWwindow* window,int width,int height);
    static void onMousePosition(GLFWwindow* window, double mouseX, double mouseY);
    static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
    static void onScroll(GLFWwindow* window, double deltaX, double deltaY);
    static void OnKeyBoard(GLFWwindow* window,float deltaTime);

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
    CameraState camera;
    InputState input;

};

#endif //GAMEENGINE_GLAPPLICATION_H