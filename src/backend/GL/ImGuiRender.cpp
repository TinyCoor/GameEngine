//
// Created by 12132 on 2021/10/30.
//

#include "ImGuiRender.h"
#include "glad.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void ImGuiRender::update() {

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui::Begin("My First Tool", &ToolOpen, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open..", "Ctrl+O")) {
                /* Do stuff */
            }
            if (ImGui::MenuItem("Save", "Ctrl+S"))   {
                /* Do stuff */
            }
            if (ImGui::MenuItem("Close", "Ctrl+W"))  { ToolOpen = false; }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();

        ImGui::SliderFloat("x", &position.x, -1.0f, 1.0f);
        ImGui::SliderFloat("y", &position.y, -1.0f, 1.0f);
        ImGui::SliderFloat("z", &position.z, -1.0f, 1.0f);

    }
    ImGui::End();

//    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
//    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


}

void ImGuiRender::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplOpenGL3_Init("#version 450");
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    ImGui::StyleColorsClassic();
}

void ImGuiRender::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}

ImGuiRender::~ImGuiRender() {
    shutdown();
}
