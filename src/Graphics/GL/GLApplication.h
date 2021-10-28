//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_GLAPPLICATION_H
#define GAMEENGINE_GLAPPLICATION_H

#endif //GAMEENGINE_GLAPPLICATION_H

class GLFWwindow;
class GLApplication{
public:
    void initGLFW();

    void display();

    void shutdownGLFW();

private:

    GLFWwindow* window;
};
