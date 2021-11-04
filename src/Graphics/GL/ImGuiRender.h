//
// Created by 12132 on 2021/10/30.
//

#ifndef GAMEENGINE_IMGUIRENDER_H
#define GAMEENGINE_IMGUIRENDER_H

class GLFWwindow;
class ImGuiRender {
public:
    ImGuiRender()=default;
    ~ImGuiRender();
    void init(GLFWwindow* window);

    void update();
    void shutdown();

};


#endif //GAMEENGINE_IMGUIRENDER_H