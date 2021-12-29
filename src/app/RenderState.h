//
// Created by 12132 on 2021/11/7.
//

#ifndef GAMEENGINE_RENDERSTATE_H
#define GAMEENGINE_RENDERSTATE_H

#include <glm/glm.hpp>
namespace render::backend::vulkan {
struct RenderState {
  glm::mat4 world;
  glm::mat4 view;
  glm::mat4 proj;
  glm::mat4 invProj;
  glm::vec3 cameraPosWS;
  glm::vec4 cameraParams;

  float lerpUserValues{0.0f};
  float userMetalness{0.0f};
  float userRoughness{0.0f};
  int currentEnvironment{0};
};
}
#endif //GAMEENGINE_RENDERSTATE_H
