//
// Created by y123456 on 2021/10/10.
//

#include "Application.h"

int main(){
   if(!glfwInit())
       return EXIT_FAILURE;
   try {
       Application app;
       app.run();
   }catch (const std::exception& e){
       std::cerr<<e.what()<<std::endl;
       glfwTerminate();
       return EXIT_FAILURE;
   }
    glfwTerminate();
    return EXIT_SUCCESS;
}

