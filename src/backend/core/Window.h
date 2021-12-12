//
// Created by 12132 on 2021/12/11.
//

#ifndef GAMEENGINE_SRC_BACKEND_CORE_WINDOW_H
#define GAMEENGINE_SRC_BACKEND_CORE_WINDOW_H
#include <cstdint>
#include <functional>

class GLFWwindow;
class Window
{
public:
    using frame_buffer_resize_cb = std::function<void(GLFWwindow*,int width,int height)>;
    using mouse_move_cb = std::function<void(GLFWwindow*,double mouseX,double mouseY)>;
    using mouse_scroll_cb = std::function<void(GLFWwindow*,double deltaX, double deltaY)>;
    using mouse_button_cb = std::function<void(GLFWwindow* window, int button, int action, int mods)>;
    using keyboard_cb = std::function<void(GLFWwindow* window,float deltaTime)>;

    Window(const char* title,uint32_t width,uint32_t height);
    ~Window();
    virtual void init();
    virtual void MainLoop() = 0;

    void RegisterFrameBufferResizeEvent(frame_buffer_resize_cb cb);
    void RegisterMouseMoveEvent(mouse_move_cb cb);
    void RegisterMouseButtonEvent(mouse_button_cb cb);
    void RegisterMouseScrollEvent(mouse_scroll_cb cb);
    void RegisterKeyboardScrollEvent(keyboard_cb  cb);


private:
    GLFWwindow* window;

};

#endif //GAMEENGINE_SRC_BACKEND_CORE_WINDOW_H
