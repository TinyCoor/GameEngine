//
// Created by y123456 on 2021/10/11.
//
#include "VulkanApplication.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderScene.h"
#include "VulkanRender.h"
#include "VulkanImGuiRender.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <functional>
#include <volk.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>



void Application::initWindow() {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GL_RED_BITS,mode->redBits);
    glfwWindowHint(GL_GREEN_BITS,mode->greenBits);
    glfwWindowHint(GL_BLUE_BITS,mode->blueBits);
    glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);

    window = glfwCreateWindow(mode->width,mode->height,"Vulkan", nullptr, nullptr);

    glfwSetWindowUserPointer(window,this);
    glfwSetFramebufferSizeCallback(this->window,&Application::OnFrameBufferResized);
}

void Application::run(){
    initWindow();
    initImGui();
    initVulkan();
    initVulkanSwapChain();
    initScene();
    initRenders();
    mainLoop();
    shutdownRenders();
    shutdownScene();
    shutdownSwapChain();
    shutdownImGui();
    shutdownVulkan();
    shutdownWindow();
}

Application::Application(){

}

Application::~Application(){

}



void Application::initVulkan() {
    if(context == nullptr){
        context =new VulkanContext;
        context->init(window);
    }
}

void Application::shutdownVulkan() {
    if(context){
        context->shutdown();
        delete context;
        context = nullptr;
    }
}

void Application::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    //TODO
    ImGui_ImplGlfw_InitForVulkan(window,true);

}


void Application::shutdownImGui() {
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Application::shutdownWindow() {
    glfwDestroyWindow(window);
    window = nullptr;
}

void Application::RenderFrame(){
    VulkanRenderFrame frame;

    if(!swapChain->Acquire(state,frame)){
        recreateSwapChain();
        return;
    }

    render->render(scene, frame);
    ImGuiRender->render(scene,frame);

    if(!swapChain->Present(frame) || windowResized){
        windowResized = false;
        recreateSwapChain();
    }

}
void Application::update()
{
    render->update(state,scene);
    //ImGui
    static float f = 0.0f;
    static int counter = 0;
    static bool show_demo_window = false;

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    ImGui::Begin("Material Parameters");

    if(ImGui::Button("Reload Shader")){
        scene->reloadShader();
        render->reload(scene);
        render->setEnvironment(scene->getHDRTexture(state.currentEnvironment));
    }

   // int oldCurrentEnvironment =state.currentEnvironment;
    if(ImGui::BeginCombo("Chose your Destiny",scene->getHDRTexturePath(state.currentEnvironment))){
        for (int i = 0; i < scene->getNumHDRTextures(); ++i) {
            bool selected = (i==state.currentEnvironment);
            if (ImGui::Selectable(scene->getHDRTexturePath(i),&selected)){
                state.currentEnvironment = i;
                render->setEnvironment(scene->getHDRTexture(state.currentEnvironment));
            }

            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Image(bakedBRDF,ImVec2(256.f,256.f));


    ImGui::Checkbox("Demo Window", &show_demo_window);

    ImGui::SliderFloat("Lerp User Material", &state.lerpUserValues, 0.0f, 1.0f);
    ImGui::SliderFloat("Metalness", &state.userMetalness, 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness", &state.userRoughness, 0.0f, 1.0f);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void Application::mainLoop() {
    if(!window)
        return;

    while (!glfwWindowShouldClose(window)){
         ImGui_ImplGlfw_NewFrame();
         ImGui::NewFrame();

         update();
        ImGui::Render();
        RenderFrame();

        glfwPollEvents();
    }

    vkDeviceWaitIdle(context->device);
}


void Application::shutdownRenders() {
    if(render){
        render->shutdown();
        delete render;
        render = nullptr;
    }

    ImGuiRender->shutdown();


}

void Application::initRenders() {
    if(!render){
        render = new VulkanRender(context,swapChain->getExtent(),swapChain->getDescriptorSetLayout(),swapChain->getRenderPass());
        render->init(scene);
    }

    render->setEnvironment(scene->getHDRTexture( state.currentEnvironment));

    if (!ImGuiRender){
        ImGuiRender = new VulkanImGuiRender(context,swapChain->getExtent(),swapChain->getNoClearRenderPass());
         ImGuiRender->init(scene,swapChain);
    }
}


void Application::initVulkanSwapChain() {
  //TODO
    if (!swapChain){
        swapChain= std::shared_ptr<VulkanSwapChain>(new VulkanSwapChain(context, sizeof(RenderState)));
    }

    int width,height;
    glfwGetWindowSize(window,&width,&height);
    swapChain->init(width,height);
}

void Application::shutdownSwapChain() {
   //TODO
   swapChain->shutdown();
}

void Application::initScene() {
    scene = new VulkanRenderScene(context);
    scene->init();
}

void Application::shutdownScene() {
//    scene->shutdown();
    if(scene){
        delete scene;
        scene = nullptr;
    }

}

void Application::recreateSwapChain() {
    int width =0;
    int height = 0;
    while (width ==0 || height == 0){
        glfwGetFramebufferSize(window,&width,&height);
        glfwPollEvents();
    }
    vkDeviceWaitIdle(context->device);

    glfwGetWindowSize(window,&width,&height);
    swapChain->reinit(width,height);
    render->resize(swapChain);
    ImGuiRender->resize(swapChain);

}


void Application::OnFrameBufferResized(GLFWwindow *window, int width, int height) {
    Application* app =(Application*)glfwGetWindowUserPointer(window);
    assert(app != nullptr);
    app->windowResized= true;
}
