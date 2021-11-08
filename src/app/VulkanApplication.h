//
// Created by y123456 on 2021/10/11.
//

#ifndef GAMEENGINE_VULKANAPPLICATION_H
#define GAMEENGINE_VULKANAPPLICATION_H
#include "VulkanContext.h"
#include "RenderState.h"
#include <volk.h>
#include <memory>

class GLFWwindow;
class VulkanRender;
class VulkanRenderScene;
class VulkanSwapChain;
class VulkanImGuiRender;

class Application{
public:

    Application();
    ~Application();
    void run();

private:

    void initWindow();
    void initVulkan();
    void shutdownVulkan();

    void initVulkanSwapChain();
    void shutdownSwapChain();

    void initScene();
    void shutdownScene();

    void initImGui();
    void shutdownImGui();

    void initRenders();
    void shutdownRenders();

    void mainLoop();
    void update();
    void RenderFrame();
    void shutdownWindow();

    void recreateSwapChain();

    static void OnFrameBufferResized(GLFWwindow* window,int width,int height);

private:
    GLFWwindow* window{nullptr};
    bool windowResized = false;

    VulkanContext* context;
    VulkanRenderScene* scene{nullptr};
    RenderState state;

    VulkanRender* render{nullptr};
    VulkanImGuiRender* ImGuiRender{nullptr};

    std::shared_ptr<VulkanSwapChain> swapChain;



};


#endif //GAMEENGINE_VULKANAPPLICATION_H
