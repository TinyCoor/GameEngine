//
// Created by y123456 on 2021/10/11.
//

#ifndef GAMEENGINE_VULKANAPPLICATION_H
#define GAMEENGINE_VULKANAPPLICATION_H
#include "RenderState.h"
#include <volk.h>
#include <memory>
#include <imgui.h>
#include "../backend/driver.h"
#include "../backend/Vulkan/VulkanSwapChain.h"
#include "../backend/Vulkan/VulkanImGuiRender.h"
#include "../backend/Vulkan/Scene.h"
#include "VulkanRender.h"
#include "../backend/Vulkan/driver.h"

class GLFWwindow;
using namespace render::backend::vulkan;



struct CameraState {
  double phi{0.0f};
  double theta{0.0f};
  double radius{2.0f};
  glm::vec3 target;
};

struct InputState {
  const double rotationSpeed{0.01};
  const double scrollSpeed{1.5};
  bool rotating{false};
  double lastMouseX{0.0};
  double lastMouseY{0.0};
};

class Application {
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

  static void OnFrameBufferResized(GLFWwindow *window, int width, int height);
  static void onMousePosition(GLFWwindow *window, double mouseX, double mouseY);
  static void onMouseButton(GLFWwindow *window, int button, int action, int mods);
  static void onScroll(GLFWwindow *window, double deltaX, double deltaY);

private:
  GLFWwindow *window{nullptr};
  bool windowResized = false;

  render::backend::Driver* driver{nullptr};

  Scene* sponza_scene{nullptr};
  VulkanRenderScene *scene{nullptr};
  RenderState state;
  //TODO remove
  static inline ImTextureID bakedBRDF{nullptr};

  VulkanRender *render{nullptr};
  ImGuiRender *imGuiRender{nullptr};
  VulkanSwapChain* swapChain;

  //TODO move to anther
  CameraState camera;
  InputState input;

};


#endif //GAMEENGINE_VULKANAPPLICATION_H
