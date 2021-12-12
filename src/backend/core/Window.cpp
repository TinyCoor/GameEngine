//
// Created by 12132 on 2021/12/11.
//

#include "Window.h"
#include "../GL/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>

Window::Window(const char *title, uint32_t width, uint32_t height)
{
    glfwSetErrorCallback([](int error, const char *message) {
        std::cerr << message << "\n";
    });
    if (!glfwInit()) exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window;
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Create GLFW Window Failed\n";
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mode) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });

    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        std::cerr << "init glad failed\n";
    }
}


Window::~Window()
{

}
void Window::init()
{

}
void Window::RegisterFrameBufferResizeEvent(Window::frame_buffer_resize_cb cb)
{
    glfwSetFramebufferSizeCallback(window, reinterpret_cast<GLFWframebuffersizefun>(&cb));
}
void Window::RegisterMouseMoveEvent(Window::mouse_move_cb cb)
{

}
void Window::RegisterMouseScrollEvent(Window::mouse_scroll_cb cb)
{
    glfwSetScrollCallback(window, reinterpret_cast<GLFWscrollfun>(&cb));
}
void Window::RegisterKeyboardScrollEvent(Window::keyboard_cb cb)
{

}
