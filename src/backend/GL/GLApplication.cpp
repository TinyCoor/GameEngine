//
// Created by y123456 on 2021/10/22.
//


#include "GLApplication.h"
#include "imgui.h"
#include <iostream>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "VAO.h"
#include "Buffer/GLBuffer.hpp"
#include "GLShader.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <chrono>

void GLApplication::shutdownGLFW() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool GLApplication::render() {

    float deltaTime =0;
    float lastFrame = 0;

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();

    const float rotationSpeed = 0.1f;
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    const glm::vec3 &up = {0.0f, 0.0f, 1.0f};
    const glm::vec3 &zero = {0.0f, 0.0f, 0.0f};

    const float aspect = 1920 / (float) 1080;
    const float zNear = 0.1f;
    const float zFar = 100000.0f;

    struct State{
        glm::mat4 world;
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec3 cameraPosWS;
    };
    State state;
    glm::vec3 cameraPos;

    GLShader vertShader(ShaderKind::vertex);
    vertShader.compileFromFile("../../assets/shaders/GL/trace.vert");
    GLShader fragShader(ShaderKind::fragment);
    fragShader.compileFromFile("../../assets/shaders/GL/trace.frag");

    GLProgram traceProgram;
    traceProgram.link(vertShader,fragShader);
    std::vector<float> vertices{
        -1.f, 1.f,
        -1.f, -1.f,
        1.f, 1.f,
        1.f, -1.f
    };
    Vao vao;
    vao.Bind();
    GLBuffer<GL_ARRAY_BUFFER> cubeVertices;
    cubeVertices.Bind();
    cubeVertices.CopyToGPU(vertices.data(),vertices.size()* sizeof(float),GL_MAP_WRITE_BIT);

    GLVertexAttribute attribute(0,2, sizeof(float),0);
    attribute();


    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = glfwGetTime();
        traceProgram.SetUniformFloat("uf_time",time);
        traceProgram.SetVec2f("uv2_resolution",1920.0,1080.f);
        traceProgram.use();

        cameraPos.x = static_cast<float>(glm::cos(camera.phi) * glm::cos(camera.theta) * camera.radius);
        cameraPos.y = static_cast<float>(glm::sin(camera.phi) * glm::cos(camera.theta) * camera.radius);
        cameraPos.z = static_cast<float>(glm::sin(camera.theta) * camera.radius);

        state.world = glm::mat4(1.0f);
        state.view  = glm::lookAt(cameraPos, zero, up);
        state.proj = glm::perspective(glm::radians(60.0f), aspect, zNear, zFar);
        state.proj[1][1] *= -1;
        state.cameraPosWS = cameraPos;

        glDrawArrays(GL_TRIANGLE_STRIP,0,8);

        imGuiRender_->update();
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

    glfwSetWindowUserPointer(window, this);
    imGuiRender_->init(window);
    return true;
}



void GLApplication::shutdown() {
    shutdownGLFW();
    imGuiRender_->shutdown();
}



void GLApplication::OnFrameBufferResized(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void GLApplication::onMousePosition(GLFWwindow *window, double mouseX, double mouseY) {
    GLApplication* application = reinterpret_cast<GLApplication*> (glfwGetWindowUserPointer(window));

    if (application->input.rotating)
    {
        double deltaX = mouseX - application->input.lastMouseX;
        double deltaY = mouseY - application->input.lastMouseY;

        application->camera.phi -= deltaX * application->input.rotationSpeed;
        application->camera.theta += deltaY * application->input.rotationSpeed;

        application->camera.phi = std::fmod(application->camera.phi, glm::two_pi<double>());
        application->camera.theta = std::clamp<double>(application->camera.theta, -glm::half_pi<double>(), glm::half_pi<double>());
    }

    application->input.lastMouseX = mouseX;
    application->input.lastMouseY = mouseY;
}

void GLApplication::onScroll(GLFWwindow *window, double deltaX, double deltaY) {
    GLApplication* app = reinterpret_cast<GLApplication*> (glfwGetWindowUserPointer(window));
    app->camera.radius -= deltaY * app->input.scrollSpeed;
}

void GLApplication::OnKeyBoard(GLFWwindow *window,float deltaTime) {
    GLApplication* app = reinterpret_cast<GLApplication*> (glfwGetWindowUserPointer(window));
    //TODO
}

void GLApplication::onMouseButton(GLFWwindow *window, int button, int action, int mods) {
    GLApplication* app = reinterpret_cast<GLApplication*> (glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
        app->input.rotating = (action == GLFW_PRESS);
}

