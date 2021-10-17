cmake_minimum_required(VERSION 3.12)
#



#aux_source_directory(./ SRC)
file(GLOB SOURCE ../extern/volk/volk.c
        app/*.h app/*.cpp
        )
add_executable(niagara
        ${SOURCE}
        )


message(STATUS ${ShaderC_LIBRARIES})

add_compile_definitions(VK_USE_PLATFORM_WIN32)
target_include_directories(niagara PRIVATE ../extern/glfw/include)
target_include_directories(niagara PRIVATE ../extern/glm)
target_include_directories(niagara PRIVATE ../extern/assimp/include)
target_include_directories(niagara PRIVATE ../cmake-build-debug/extern/assimp/include)
target_include_directories(niagara PRIVATE ../extern/assimp/contrib/stb)
target_include_directories(niagara PRIVATE ../extern/tinyobjloader)
target_include_directories(niagara PRIVATE ../extern/shaderc/include)
target_include_directories(niagara PRIVATE ../extern/volk)
target_link_directories(niagara PRIVATE ${ShaderC_LIBRARIES})
target_link_libraries(niagara PRIVATE glfw)
target_link_libraries(niagara PRIVATE assimp)
target_link_libraries(niagara PRIVATE shaderc_shared)
target_link_libraries(niagara PRIVATE opengl32 volk )

