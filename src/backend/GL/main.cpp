//
// Created by y123456 on 2021/10/28.
//
#include "GLApplication.h"
#include "GLTexture.h"
#include "VAO.h"
#include <GLFW/glfw3.h>
const int width =1920;
const int height = 1080;


GLFWwindow* InitGLFW(){

  glfwSetErrorCallback([](int error, const char *message) {
    std::cerr << message << "\n";
  });
  if (!glfwInit()) exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window;
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

  glfwSetFramebufferSizeCallback(window, &GLApplication::OnFrameBufferResized);
  glfwSetCursorPosCallback(window, &GLApplication::onMousePosition);
  glfwSetScrollCallback(window, &GLApplication::onScroll);
  glfwSetMouseButtonCallback(window, &GLApplication::onMouseButton);
  if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
    std::cerr << "init glad failed\n";
    return nullptr;
  }
  return window;

}
int main(){

    auto window = InitGLFW();
    GLApplication app(window);
    app.run();

    return 0;

}