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
#include <glm/gtc/matrix_transform.hpp>
#include "GLProgram.h"
#include "GLContext.h"
#include "GLTexture.h"


const int width =1920;
const int height = 1080;


float vertices[] = {
        // positions          // colors           // texture coords
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left
};

unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
};

//float texCoords[] = {
//        0.0f, 0.0f, // 左下角
//        1.0f, 0.0f, // 右下角
//        0.5f, 1.0f // 上中
//};


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
    GLBuffer<GL_ELEMENT_ARRAY_BUFFER> ebo;
    ebo.Bind();
    ebo.CopyToGPU((uint8_t*)indices, sizeof(indices), GL_MAP_WRITE_BIT);


    GLShader vertShader(ShaderKind::vertex);
    vertShader.compileFromFile("../../assets/shaders/triangle.vert");
    GLShader fragShader(ShaderKind::fragment);
    fragShader.compileFromFile("../../assets/shaders/triangle.frag");

    GLProgram program;
    program.link(vertShader,fragShader);


    GLTexture<GL_TEXTURE_2D> texture;
    texture.loadFromFile("../../assets/textures/wall.jpg");

    VertexInputAttribute<GL_FLOAT> vertexInputAttribute(0,3,8* sizeof(float ));
    vertexInputAttribute.SetVertexInputAttribute(GL_FALSE,(void*)0);

    VertexInputAttribute<GL_FLOAT> vertexInputAttribute2(1,3,8 * sizeof(float ));
    vertexInputAttribute2.SetVertexInputAttribute(GL_FALSE,(void*)(3 * sizeof(float)));

    VertexInputAttribute<GL_FLOAT> vertexInputAttribute3(2,2,8 * sizeof(float ));
    vertexInputAttribute3.SetVertexInputAttribute(GL_FALSE,(void*)(6 * sizeof(float)));

    vao.UnBind();


    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        imGuiRender_->update();

        auto pos = imGuiRender_->GetPosition();
        program.use();

        glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        transform = glm::translate(transform, pos);
  //      transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
        program.SetUniformMatrix4fv("transform", 1, GL_FALSE, transform);
        //TODO

        program.SetUniformVec4f("ourColor",1.0,0.0,0.0,1.0);
        vao.Bind();
        texture.Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


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
