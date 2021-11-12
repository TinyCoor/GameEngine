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
#include "GLTexture.h"


const int width =1920;
const int height = 1080;
Camera camera(glm::vec3(0.0f, 0.0f, 6.0f));

float vertices[] = {
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
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

    GLShader vertShader(ShaderKind::vertex);
    vertShader.compileFromFile("../../assets/shaders/GL/light_cube.vert");
    GLShader fragShader(ShaderKind::fragment);
    fragShader.compileFromFile("../../assets/shaders/GL/light_cube.frag");

    GLProgram lightCubeProgram;
    lightCubeProgram.link(vertShader,fragShader);

    GLShader colorVertShader(ShaderKind::vertex);
    colorVertShader.compileFromFile("../../assets/shaders/GL/color.vert");
    GLShader colorFragShader(ShaderKind::fragment);
    colorFragShader.compileFromFile("../../assets/shaders/GL/color.frag");

    GLProgram lightingProgram;
    lightingProgram.link(colorVertShader,colorFragShader);
    Vao cubeVao;
    GLBuffer<GL_ARRAY_BUFFER> vbo;
    vbo.Bind();
    vbo.CopyToGPU((uint8_t*)vertices,sizeof(vertices), GL_DYNAMIC_STORAGE_BIT);
    cubeVao.Bind();

    VertexInputAttribute<GL_FLOAT> vertexInputAttribute(0,3,3 * sizeof(float));
    vertexInputAttribute.SetVertexInputAttribute(GL_FALSE,(void*)0);

    Vao lightCubeVao;
    lightCubeVao.Bind();
    vbo.Bind();


    VertexInputAttribute<GL_FLOAT> vertexInputAttribute2(0,3,3 * sizeof(float));
    vertexInputAttribute2.SetVertexInputAttribute(GL_FALSE,(void*)0);


    glEnable(GL_DEPTH_TEST);


    // lighting
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        imGuiRender_->update();

        auto pos = imGuiRender_->GetPosition();


        lightingProgram.use();
        lightingProgram.SetUniformVec3f("objectColor", 1.0f, 0.5f, 0.31f);
        lightingProgram.SetUniformVec3f("lightColor",  1.0f, 1.0f, 1.0f);
//
        glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)1920 / (float)1080, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
//
        lightingProgram.SetUniformMatrix4fv("projection",1,GL_FALSE, projection);
        lightingProgram.SetUniformMatrix4fv("view",1,GL_FALSE, view);
//
//        // world transformation
         glm::mat4 model = glm::mat4(1.0f);
        lightingProgram.SetUniformMatrix4fv("model",1,GL_FALSE, model);

        cubeVao.Bind();
        glDrawArrays(GL_TRIANGLES,0,36);

        lightCubeProgram.use();
        lightCubeProgram.SetUniformMatrix4fv("projection",1,GL_FALSE, projection);
        lightCubeProgram.SetUniformMatrix4fv("view",1,GL_FALSE,view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        lightCubeProgram.SetUniformMatrix4fv("model",1,GL_FALSE, model);

        cubeVao.Bind();
        glDrawArrays(GL_TRIANGLES,0,36);


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
