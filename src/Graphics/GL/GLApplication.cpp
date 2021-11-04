//
// Created by y123456 on 2021/10/22.
//


#include "GLApplication.h"
#include "imgui.h"
#include <iostream>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "VAO.h"
#include "GLBuffer.hpp"
#include "GLShader.h"
#include <GLFW/glfw3.h>
#include "GLProgram.h"
#include "GLContext.h"


const int width =1920;
const int height = 1080;


float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
};


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

    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        std::cerr << "init glad failed\n";
        exit(-1);
    }
}

void GLApplication::shutdownGLFW() {
    glfwDestroyWindow(window);
   // glfwTerminate();
}

bool GLApplication::render() {

    Vao vao;
    GLBuffer<GL_ARRAY_BUFFER> vbo;
    vao.Bind();
    vbo.Bind();
    vbo.CopyToGPU((uint8_t*)vertices,sizeof(vertices), GL_MAP_WRITE_BIT);

    GLShader vertShader(ShaderKind::vertex);
    vertShader.compileFromFile("../../assets/shaders/triangle.vert");
    GLShader fragShader(ShaderKind::fragment);
    fragShader.compileFromFile("../../assets/shaders/triangle.frag");

    GLProgram program;
    program.link(vertShader,fragShader);

    VertexInputAttribute<GL_FLOAT> vertexInputAttribute(0,3,3* sizeof(float ));
    vertexInputAttribute.SetVertexInputAttribute(GL_FALSE,(void*)0);
    vao.UnBind();

    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        imGuiRender_->update();

        //TODO
        program.use();
        vao.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);


        glfwSwapBuffers(window);
    }
    return true;
}

void GLApplication::run() {
    init();
    render();
    shutdown();
}

bool GLApplication::init() {
    initGLFW();
    imGuiRender_->init(window);
    return true;
}



void GLApplication::shutdown() {
    shutdownGLFW();
//    imGuiRender_->shutdown();
}
