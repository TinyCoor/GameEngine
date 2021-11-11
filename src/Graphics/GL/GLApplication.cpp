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
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

// world space positions of our cubes
glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3 (2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
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

    VertexInputAttribute<GL_FLOAT> vertexInputAttribute(0,3,5* sizeof(float ));
    vertexInputAttribute.SetVertexInputAttribute(GL_FALSE,(void*)0);

    VertexInputAttribute<GL_FLOAT> vertexInputAttribute2(1,2,5 * sizeof(float ));
    vertexInputAttribute2.SetVertexInputAttribute(GL_FALSE,(void*)(3 * sizeof(float)));

    GLShader vertShader(ShaderKind::vertex);
    vertShader.compileFromFile("../../assets/shaders/GL/camera.vert");
    GLShader fragShader(ShaderKind::fragment);
    fragShader.compileFromFile("../../assets/shaders/GL/camera.frag");

    GLProgram program;
    program.link(vertShader,fragShader);


    GLTexture<GL_TEXTURE_2D> texture;
    texture.loadFromFile("../../assets/textures/GL/container.jpg");
    GLTexture<GL_TEXTURE_2D> texture2;
    texture.loadFromFile("../../assets/textures/GL/awesomeface.png");

    vao.UnBind();

    program.use();
    program.SetUniformInt("texture1",0);
    program.SetUniformInt("texture2",1);

    glm::mat4 projection = glm::perspective(glm::radians(45.f),1920/(float)1080,1.f,1000.f);
    program.SetUniformMatrix4fv("projection",1,GL_FALSE,projection);



    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        imGuiRender_->update();

        auto pos = imGuiRender_->GetPosition();

        texture.Bind();
        texture2.Bind();
        program.use();
        glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        float radius =10.f;
        float camX = sin(glfwGetTime()) *radius;
        float camZ = cos(glfwGetTime()) * radius;
        view = glm::lookAt(glm::vec3(camX,0.f,camZ),
                          glm::vec3(0.0f, 0.0f, 0.0f),
                          glm::vec3(0.0f, 1.0f, 0.0f));
        program.SetUniformMatrix4fv("view",1,GL_FALSE,view);

        vao.Bind();

        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            program.SetUniformMatrix4fv("model",1,GL_FALSE,model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


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
