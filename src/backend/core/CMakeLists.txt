cmake_minimum_required(VERSION 3.12)

set(Target core)

add_library(${Target} STATIC
        file.cpp
        Vertex.cpp Window.cpp Window.h)

target_include_directories(${Target} PRIVATE ../../../extern/volk)
target_include_directories(${Target} PRIVATE  ../../../extern/glfw/include)
target_include_directories(${Target} PRIVATE  ../../../extern/glm)
target_include_directories(${Target} PRIVATE  ../../../extern/imgui/backends)
target_include_directories(${Target} PRIVATE  ../../../extern/imgui)
target_include_directories(${Target} PRIVATE ../../../extern/assimp/contrib/stb)
target_include_directories(${Target} PRIVATE ../../../extern/assimp/include)
target_include_directories(${Target} PRIVATE ../../../extern/tinyobjloader)
target_include_directories(${Target} PRIVATE ${PROJECT_ROOT}/cmake-build-debug/extern/assimp/include)

target_link_libraries(${Target} PRIVATE
        assimp
        tinyobjloader)

