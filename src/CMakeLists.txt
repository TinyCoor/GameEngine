cmake_minimum_required(VERSION 3.12)
find_package(Vulkan REQUIRED)


set(ROOT_FOLD ${CMAKE_CURRENT_LIST_DIR})

if (Vulkan_FOUND)
    message(STATUS "Find Vulkan")
    message(STATUS ${Vulkan_INCLUDE_DIRS} )
    include_directories(${Vulkan_INCLUDE_DIRS})
endif ()

add_executable(niagara
        app/VulkanTexture.cpp
        app/VulkanUtils.cpp
        app/VulkanMesh.cpp
        app/VulkanGraphicsPipelineBuilder.cpp
        app/VulkanRender.cpp
        app/VulkanRenderScene.cpp
        app/VulkanRenderContext.cpp
        app/VulkanShader.cpp
        app/VulkanTexture.cpp
        app/pbrRender.cpp
        app/VulkanApplication.cpp
        app/VulkanDescriptorSetLayoutBuilder.cpp
        app/VulkanRenderPassBuilder.cpp
        app/VulkanPipelineLayoutBuilder.cpp
        app/VulkanCubemapRender.cpp

        ../extern/volk/volk.c
        ../extern/imgui/imgui.cpp
        ../extern/imgui/imgui_draw.cpp
        ../extern/imgui/imgui_widgets.cpp
        ../extern/imgui/imgui_tables.cpp
        ../extern/imgui/imgui_demo.cpp
        ../extern/imgui/backends/imgui_impl_glfw.cpp
        ../extern/imgui/backends/imgui_impl_vulkan.cpp
         app/config.h 
         app/ResourceManager.cpp 
         app/ResourceManager.h)


message(STATUS ${ShaderC_LIBRARIES})

add_compile_definitions(VK_USE_PLATFORM_WIN32)
target_include_directories(niagara PRIVATE ../extern/glfw/include)
target_include_directories(niagara PRIVATE ../extern/imgui)
target_include_directories(niagara PRIVATE ../extern/imgui/backends)
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
target_link_libraries(niagara PRIVATE opengl32 )

add_subdirectory(Graphics/GL)

