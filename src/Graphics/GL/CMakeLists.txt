cmake_minimum_required(VERSION 3.12)

#aux_source_directory(./ SRC)

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
        )


target_include_directories(OpenGL PRIVATE ../extern/glfw/include)
target_include_directories(OpenGL PRIVATE ../extern/glm)
target_link_libraries(OpenGL PRIVATE opengl32 )
target_link_libraries(OpenGL PRIVATE glfw)

