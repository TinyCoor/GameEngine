cmake_minimum_required(VERSION 3.12)

find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    message(STATUS "Find Vulkan")
    message(STATUS ${Vulkan_INCLUDE_DIRS} )
    include_directories(${Vulkan_INCLUDE_DIRS})
endif ()



add_executable(niagara
        backend/Vulkan/VulkanTexture.cpp
        backend/Vulkan/VulkanUtils.cpp
        backend/Vulkan/VulkanMesh.cpp
        backend/Vulkan/VulkanGraphicsPipelineBuilder.cpp
        app/VulkanRender.cpp
        app/VulkanRenderScene.cpp
        backend/Vulkan/Device.cpp
        backend/Vulkan/VulkanShader.cpp
        backend/Vulkan/VulkanTexture.cpp
        app/pbrRender.cpp
        app/VulkanApplication.cpp
        backend/Vulkan/VulkanDescriptorSetLayoutBuilder.cpp
        backend/Vulkan/VulkanRenderPassBuilder.cpp
        backend/Vulkan/VulkanPipelineLayoutBuilder.cpp
        backend/Vulkan/VulkanCubemapRender.cpp
        backend/Vulkan/VulkanSwapChain.cpp
        ../extern/volk/volk.c
        ../extern/imgui/imgui.cpp
        ../extern/imgui/imgui_draw.cpp
        ../extern/imgui/imgui_widgets.cpp
        ../extern/imgui/imgui_tables.cpp
        ../extern/imgui/imgui_demo.cpp
        ../extern/imgui/backends/imgui_impl_glfw.cpp
        ../extern/imgui/backends/imgui_impl_vulkan.cpp
         app/config.h
        backend/Vulkan/VulkanResourceManager.cpp
        backend/Vulkan/VulkanResourceManager.h
        Entity/camera.cpp
        Entity/camera.h
        app/RenderState.h
        backend/Vulkan/VulkanImGuiRender.cpp
        backend/Vulkan/VulkanImGuiRender.h
        backend/Vulkan/VulkanTexture2DRender.cpp
        backend/Vulkan/VulkanTexture2DRender.h
        Entity/light.cpp
        Entity/light.h
        Entity/material.h
        backend/driver.h
        backend/driver.cpp
        backend/core/file.cpp
        backend/core/file.h
        backend/core/Vertex.h
        backend/core/Vertex.cpp
        backend/Vulkan/driver.cpp
        backend/Vulkan/driver.h
        backend/Vulkan/platform.cpp
        backend/Vulkan/platform.h
        backend/core/typedef.h
        backend/Vulkan/VulkanRenderPassCache.cpp
        backend/Vulkan/VulkanRenderPassCache.h
        backend/core/utils.h
        backend/Vulkan/PipelineLayoutCache.cpp
        backend/Vulkan/PipelineLayoutCache.h
        backend/Vulkan/DescriptorSetLayoutCache.cpp
        backend/Vulkan/context.cpp backend/Vulkan/context.h backend/Vulkan/auxiliary.h backend/Vulkan/VulkanResourceCache.cpp backend/Vulkan/VulkanResourceCache.h backend/Vulkan/PipelineCache.cpp backend/Vulkan/PipelineCache.h)


message(STATUS ${ShaderC_LIBRARIES})

target_compile_definitions(niagara PRIVATE VK_USE_PLATFORM_WIN32)
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

add_subdirectory(backend/core)
add_subdirectory(backend/GL)

