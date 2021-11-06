//
// Created by 12132 on 2021/10/30.
//

#ifndef GAMEENGINE_IMGUIRENDER_H
#define GAMEENGINE_IMGUIRENDER_H
#include <glm/glm.hpp>

class GLFWwindow;
class ImGuiRender {
public:
    ImGuiRender()=default;
    ~ImGuiRender();
    void init(GLFWwindow* window);

    void update();
    void shutdown();

    glm::vec3 GetPosition()const {return position;}
    glm::vec3 GetRotation() const{ return rotation;}

private:
    bool ToolOpen = true;
    glm::vec3 position;
    glm::vec3 rotation;

};


#endif //GAMEENGINE_IMGUIRENDER_H
