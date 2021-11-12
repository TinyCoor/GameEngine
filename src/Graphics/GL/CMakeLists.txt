cmake_minimum_required(VERSION 3.12)


add_executable(OpenGL

        GLApplication.cpp
            GLShader.cpp
            GLMesh.cpp
            GLBuffer.hpp
            GLTexture.cpp
            VAO.cpp
            GLProgram.cpp
            Texture2D.cpp
            TextureCubeMap.cpp
            main.cpp
            glad.c

        ../../../extern/imgui/imgui.cpp
        ../../../extern/imgui/imgui_draw.cpp
        ../../../extern/imgui/imgui_demo.cpp
        ../../../extern/imgui/imgui_tables.cpp
           ../../../extern/imgui/imgui_widgets.cpp
           ../../../extern/imgui/backends/imgui_impl_glfw.cpp
           ../../../extern/imgui/backends/imgui_impl_opengl3.cpp
        ImGuiRender.cpp
        ImGuiRender.h GObject.hpp GLQuery.cpp GLQuery.h GLSampler.cpp GLSampler.h Event.cpp Event.h)

target_include_directories(niagara PRIVATE ../../../extern/volk)
target_include_directories(OpenGL PRIVATE  ../../../extern/glfw/include)
target_include_directories(OpenGL PRIVATE  ../../../extern/glm)
target_include_directories(OpenGL PRIVATE  ../../../extern/imgui/backends)
target_include_directories(OpenGL PRIVATE  ../../../extern/imgui)

target_include_directories(OpenGL PRIVATE ../../../extern/assimp/contrib/stb)

target_link_libraries(OpenGL PRIVATE opengl32 )
target_link_libraries(OpenGL PRIVATE glfw)

