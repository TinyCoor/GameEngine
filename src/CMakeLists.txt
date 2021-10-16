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

#aux_source_directory(./ SRC)
add_executable(niagara
        app/pbrRender.cpp
        app/VulkanRender.cpp
        app/VulkanRender.h
        app/VulkanApplication.cpp
        app/VulkanApplication.h
        app/Macro.h
        app/VulkanMesh.cpp
        app/VulkanUtils.h
        app/VulkanUtils.cpp
        app/VulkanRenderScene.cpp
        )
target_include_directories(niagara PRIVATE ../extern/glfw/include)
target_include_directories(niagara PRIVATE ../extern/vulkan/include)
target_include_directories(niagara PRIVATE ../extern/glm)
target_include_directories(niagara PRIVATE ../extern/assimp/include)
target_include_directories(niagara PRIVATE ../cmake-build-debug/extern/assimp/include)
target_include_directories(niagara PRIVATE ../extern/assimp/contrib/stb)
target_link_directories(niagara PRIVATE ${Vulkan_LIB_PATH})
target_link_libraries(niagara PRIVATE vulkan-1)
target_link_libraries(niagara PRIVATE glfw)
target_link_libraries(niagara PRIVATE assimp)
target_link_libraries(niagara PRIVATE opengl32)

