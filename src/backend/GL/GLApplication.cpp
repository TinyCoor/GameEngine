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

#include "GLProgram.h"
#include "GLContext.h"
#include "GLTexture.h"
#include "Buffer/GLUniformBuffer.h"
#include "Buffer/GLSSBO.hpp"
#include "GLMesh.h"

const int width =1920;
const int height = 1080;


float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
};


GLApplication::GLApplication() {
}



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

    glfwSetWindowUserPointer(window,this);
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
        exit(-1);
    }


}

void GLApplication::shutdownGLFW() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool GLApplication::render() {

    GLShader vertShader(ShaderKind::vertex);
    vertShader.compileFromFile("../../assets/shaders/GL/pbr.vert");
    GLShader fragShader(ShaderKind::fragment);
    fragShader.compileFromFile("../../assets/shaders/GL/pbr.frag");

    GLProgram lightCubeProgram;
    lightCubeProgram.link(vertShader,fragShader);

    GLMesh mesh;
    mesh.loadFromFile("../../assets/models/SciFiHelmet.fbx");

    glEnable(GL_DEPTH_TEST);

    // lighting
    glm::vec3 lightPos(2.2f, 1.0f, 2.0f);

    float deltaTime =0;
    float lastFrame = 0;

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();

    const float rotationSpeed = 0.1f;
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    const glm::vec3 &up = {0.0f, 0.0f, 1.0f};
    const glm::vec3 &zero = {0.0f, 0.0f, 0.0f};

    const float aspect = width / (float) height;
    const float zNear = 0.1f;
    const float zFar = 100000.0f;

    struct State{
        glm::mat4 world;
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec3 cameraPosWS;
    };

    State state;
    GLUniformBuffer uniformBuffer(sizeof(State));
    GLRender glrender;
    glrender.init();


    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        imGuiRender_->update();

        auto pos = imGuiRender_->GetPosition();

        glm::vec3 cameraPos;
        cameraPos.x = static_cast<float>(glm::cos(camera.phi) * glm::cos(camera.theta) * camera.radius);
        cameraPos.y = static_cast<float>(glm::sin(camera.phi) * glm::cos(camera.theta) * camera.radius);
        cameraPos.z = static_cast<float>(glm::sin(camera.theta) * camera.radius);

        state.world = glm::mat4(1.0f);
        state.view = glm::lookAt(cameraPos, zero, up);
        state.proj = glm::perspective(glm::radians(60.0f), aspect, zNear, zFar);
        state.proj[1][1] *= -1;
        state.cameraPosWS = cameraPos;

        glrender.render();

//
//        lightCubeProgram.use();
//        uniformBuffer.BindUniformBuffer(0,lightCubeProgram,"RenderState");
//        void* data = uniformBuffer.Map(GL_READ_ONLY);
//        std::memcpy(data,&state, sizeof(State));
//        uniformBuffer.UnMap();
////        lightCubeProgram.SetUniformMatrix4fv("projection",1,GL_FALSE, state.proj);
////        lightCubeProgram.SetUniformMatrix4fv("view",1,GL_FALSE,state.view);
////        state.world = glm::translate(state.world, lightPos);
////        state.world = glm::scale(state.world, glm::vec3(0.2f)); // a smaller cube
////        lightCubeProgram.SetUniformMatrix4fv("view",1,GL_FALSE,state.view);
////        lightCubeProgram.SetUniformMatrix4fv("model",1,GL_FALSE, state.world);
//        mesh.draw(lightCubeProgram);


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

