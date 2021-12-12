//
// Created by 12132 on 2021/10/30.
//

#ifndef GAMEENGINE_IMGUIRENDER_H
#define GAMEENGINE_IMGUIRENDER_H
#include <glm/glm.hpp>

class GLFWwindow;
class ImGuiRender {
public:
    ImGuiRender(){};
    ~ImGuiRender();
    void init(GLFWwindow* window);

    void update();
    void shutdown();

private:
    bool ToolOpen = true;


};


#endif //GAMEENGINE_IMGUIRENDER_H
