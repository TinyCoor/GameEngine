//
// Created by y123456 on 2021/10/11.
//
#include "../backend/Vulkan/VulkanImGuiRender.h"
#include "../backend/Vulkan/SkyLight.h"
#include "Application.h"
#include "ApplicationResource.h"
#include "Render.h"
#include "RenderGraph.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <chrono>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <volk.h>

using namespace render::backend;
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
    glfwSetCursorPosCallback(window, &Application::onMousePosition);
    glfwSetMouseButtonCallback(window, &Application::onMouseButton);
    glfwSetScrollCallback(window, &Application::onScroll);
}

void Application::run() {
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
    auto vk_driver = dynamic_cast<render::backend::vulkan::VulkanDriver*>(render::backend::createDriver("","",render::backend::Api::VULKAN));
    driver = vk_driver;
}

void Application::shutdownVulkan() {
    delete driver;
    driver = nullptr;
}

void Application::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
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

    if(!swapChain->Acquire(&state,frame)){
        recreateSwapChain();
        return;
    }

    render_graph->render(sponza_scene,frame);

    RenderPassClearValue clear_values[3];
    clear_values[0].color = {0.2f, 0.2f, 0.2f, 1.0f};
    clear_values[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[2].depth_stencil = {1.0f, 0};

    RenderPassLoadOp load_ops[3] = { RenderPassLoadOp::CLEAR, RenderPassLoadOp::DONT_CARE, RenderPassLoadOp::CLEAR };
    RenderPassStoreOp store_ops[3] = { RenderPassStoreOp::STORE, RenderPassStoreOp::STORE, RenderPassStoreOp::DONT_CARE };

    RenderPassInfo info;
    info.load_ops = load_ops;
    info.store_ops = store_ops;
    info.clear_value = clear_values;

    driver->beginRenderPass(frame.command_buffer, frame.frame_buffer, &info);

    render->render(resource, light, frame);
    imGuiRender->render(frame);

    driver->endRenderPass(frame.command_buffer);

    if(!swapChain->Present(frame) || windowResized){
        windowResized = false;
        recreateSwapChain();
    }

}
void Application::update()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();

    const glm::vec3 &up = {0.0f, 0.0f, 1.0f};
    const glm::vec3 &zero = {0.0f, 0.0f, 0.0f};

    VkExtent2D extent = swapChain->getExtent();

    const float aspect = extent.width / (float) extent.height;
    const float zNear = 0.1f;
    const float zFar = 100000.0f;

    glm::vec3 cameraPos;
    cameraPos.x = static_cast<float>(glm::cos(camera.phi) * glm::cos(camera.theta) * camera.radius);
    cameraPos.y = static_cast<float>(glm::sin(camera.phi) * glm::cos(camera.theta) * camera.radius);
    cameraPos.z = static_cast<float>(glm::sin(camera.theta) * camera.radius);

    glm::vec4 cameraParams;
    cameraParams.x = zNear;
    cameraParams.y = zFar;
    cameraParams.z = 1.0/zNear;
    cameraParams.w = 1.0/zFar;


    state.world = glm::mat4(1.0f);
    state.view = glm::lookAt(cameraPos, zero, up);
    state.proj = glm::perspective(glm::radians(60.0f), aspect, zNear, zFar);
    state.proj[1][1] *= -1;
    state.cameraPosWS = cameraPos;
    state.invProj = glm::inverse(state.proj);

    static float f = 0.0f;
    static int counter = 0;
    static bool show_demo_window = false;

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    ImGui::Begin("Material Parameters");

    if(ImGui::Button("Reload Shader")){
        resource->reloadShader();
        light->setEnvironmentCubeMap(resource->getHDRIEnvironmentubeMap(state.currentEnvironment));
        light->setIrradianceCubeMap(resource->getIrridanceCubeMap(state.currentEnvironment));
    }

    if(ImGui::BeginCombo("Chose your Destiny",resource->getHDRTexturePath(state.currentEnvironment))){
        for (int i = 0; i < resource->getNumHDRTextures(); ++i) {
            bool selected = (i==state.currentEnvironment);
            if (ImGui::Selectable(resource->getHDRTexturePath(i),&selected)){
                state.currentEnvironment = i;
                light->setEnvironmentCubeMap(resource->getHDRIEnvironmentubeMap(state.currentEnvironment));
                light->setIrradianceCubeMap(resource->getIrridanceCubeMap(state.currentEnvironment));
            }

            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Checkbox("Demo Window", &show_demo_window);

    ImGui::SliderFloat("Lerp User Material", &state.lerpUserValues, 0.0f, 1.0f);
    ImGui::SliderFloat("Metalness", &state.userMetalness, 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness", &state.userRoughness, 0.0f, 1.0f);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Begin("GBuffer");

    ImTextureID base_color_id = imGuiRender->fetchTextureID(render_graph->getGBuffer().base_color);
    ImTextureID normal_id = imGuiRender->fetchTextureID(render_graph->getGBuffer().normal);
    ImTextureID depth_id = imGuiRender->fetchTextureID(render_graph->getGBuffer().depth);
    ImTextureID shading_id = imGuiRender->fetchTextureID(render_graph->getGBuffer().shading);

    ImGui::BeginGroup();
    ImGui::Image(base_color_id, ImVec2(256, 256));
    ImGui::SameLine();
    ImGui::Image(normal_id, ImVec2(256, 256));
    ImGui::Image(depth_id, ImVec2(256, 256));
    ImGui::SameLine();
    ImGui::Image(shading_id, ImVec2(256, 256));
    ImGui::EndGroup();

    ImGui::End();

    ImGui::Begin("LBuffer");

    ImTextureID diffuse_id = imGuiRender->fetchTextureID(render_graph->getLBuffer().diffuse);
    ImTextureID specular_id = imGuiRender->fetchTextureID(render_graph->getLBuffer().specular);

    ImGui::BeginGroup();
    ImGui::Image(diffuse_id, ImVec2(256, 256));
    ImGui::SameLine();
    ImGui::Image(specular_id, ImVec2(256, 256));
    ImGui::EndGroup();

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

    driver->wait();
}


void Application::shutdownRenders() {
    if(render) {
        delete render;
        render = nullptr;
    }
    imGuiRender->shutdown();
}

void Application::initRenders() {

    render = new Render(driver);
    render->init(resource);
    light->setEnvironmentCubeMap(resource->getHDRIEnvironmentubeMap(state.currentEnvironment));

    imGuiRender = new ImGuiRender(driver);
    imGuiRender->init(ImGui::GetCurrentContext());

    render_graph = new render::backend::vulkan::RenderGraph(driver);
    render_graph->init(resource,swapChain->getExtent().width,swapChain->getExtent().height);

}


void Application::initVulkanSwapChain() {
    if (!swapChain){
        swapChain= new VulkanSwapChain(driver, window,sizeof(RenderState));
    }
    int width,height;
    glfwGetWindowSize(window,&width,&height);
    swapChain->init(width,height);
}

void Application::shutdownSwapChain() {
   swapChain->shutdown();
}

void Application::initScene() {
    resource = new ApplicationResource(driver);
    resource->init();

    sponza_scene = new Scene(driver);
    sponza_scene->import("../../assets/models/pbr_sponza/Sponza.gltf");
    light = new SkyLight(driver,resource->getSkylightVertexShader(),resource->getSkylightFragmentShader());
    light->setBakedBRDFTexture(resource->getBakedBRDF());
    light->setEnvironmentCubeMap(resource->getHDRIEnvironmentubeMap(0));
    light->setIrradianceCubeMap(resource->getIrridanceCubeMap(0));
    sponza_scene->addLight(light);
}

void Application::shutdownScene() {
    delete light;
    light = nullptr;

    delete resource;
    resource = nullptr;

    delete sponza_scene;
    sponza_scene = nullptr;
}

void Application::recreateSwapChain() {
    int width =0;
    int height = 0;
    while (width ==0 || height == 0){
        glfwGetFramebufferSize(window,&width,&height);
        glfwPollEvents();
    }
    driver->wait();
    glfwGetWindowSize(window,&width,&height);
    swapChain->reinit(width,height);
    render_graph->resize(width, height);
    imGuiRender->invalidateTextureIDs();
}


void Application::OnFrameBufferResized(GLFWwindow *window, int width, int height) {
    Application* app =(Application*)glfwGetWindowUserPointer(window);
    assert(app != nullptr);

    app->windowResized= true;
}



void Application::onMousePosition(GLFWwindow* window, double mouseX, double mouseY)
{
    Application *application = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    assert(application != nullptr);

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

void Application::onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
    Application *application = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    assert(application != nullptr);

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
        application->input.rotating = (action == GLFW_PRESS);
}

void Application::onScroll(GLFWwindow* window, double deltaX, double deltaY)
{
    Application *application = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    assert(application);

    application->camera.radius -= deltaY * application->input.scrollSpeed;
}
