//
// Created by 12132 on 2021/10/30.
//

#include "ImGuiRender.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void ImGuiRender::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //TODO in this Draw ImGui
    {
        //开始绘制ImGui

        ImGui::Begin("IBinary Windows");                          // Create a window called "Hello, world!" and append into it.
        ImGui::Text("IBinary Blog");
        //ImGui::SameLine();
        ImGui::Indent(); //另起一行制表符开始绘制Button
        ImGui::Button("2222", ImVec2(100, 50));

        ImGui::End();
    }

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
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
