//
// Created by 12132 on 2021/10/30.
//

#include "ImGuiRender.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
ImGuiRender::~ImGuiRender() {
    shutdown();
}
void ImGuiRender::update() {

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
            if (ImGui::MenuItem("Close", "Ctrl+W"))  {  }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();

    }
    ImGui::End();

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


