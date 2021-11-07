//
// Created by 12132 on 2021/11/7.
//

#ifndef GAMEENGINE_RENDERSTATE_H
#define GAMEENGINE_RENDERSTATE_H

#include <glm/glm.hpp>

struct RenderState{
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 cameraPosWS;
    int currentEnvironment{0};
    float lerpUserValues {0.0f};
    float userMetalness {0.0f};
    float userRoughness {0.0f};
};

#endif //GAMEENGINE_RENDERSTATE_H
