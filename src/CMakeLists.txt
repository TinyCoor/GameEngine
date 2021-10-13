cmake_minimum_required(VERSION 3.12)
#include(../cmake/vulkan.cmake)
#find_package(Vulkan)
# Vulkan_INCLUDE_DIRS
#Vulkan_LIBRARIES
#if (Vulkan_FOUND)
#    message(STATUS "Found Vulkan Including and Linking")
#else()
#    message(STATUS "Not Found Vulkan")
#endif ()

aux_source_directory(./ SRC)
add_executable(niagara ${SRC}
        app/pbrRender.cpp
        app/utils.hpp
        app/Render.cpp
        app/Render.h app/Application.cpp
        app/Application.h
        app/macro.h
        app/vertex.cpp
        app/vertex.h
        app/render_data.cpp
        app/vulkan_utils.h
        app/vulkan_utils.cpp
        app/render_data.h)
target_include_directories(niagara PRIVATE ../extern/glfw/include)
target_include_directories(niagara PRIVATE ../extern/vulkan/include)
target_include_directories(niagara PRIVATE ../extern/glm)
target_link_directories(niagara PRIVATE ${Vulkan_LIB_PATH})
target_link_libraries(niagara PRIVATE vulkan-1)
target_link_libraries(niagara PRIVATE glfw)
target_link_libraries(niagara PRIVATE opengl32)

