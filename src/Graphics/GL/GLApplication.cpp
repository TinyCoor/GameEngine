//
// Created by y123456 on 2021/10/22.
//

#include <GLFW/glfw3.h>
#include "GLApplication.h"
#include <iostream>

const int width =1920;
const int height = 1080;

void GLApplication::initGLFW(){

    glfwSetErrorCallback([](int error, const char *message) {
        std::cerr << message << "\n";
    });
    if (!glfwInit()) exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Hello,OpenGL", nullptr, nullptr);
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
}

void GLApplication::shutdownGLFW() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void GLApplication::display() {
    while (glfwWindowShouldClose(window)){

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
