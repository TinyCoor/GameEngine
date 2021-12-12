cmake_minimum_required(VERSION 3.12)

find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    message(STATUS "Find Vulkan")
    message(STATUS ${Vulkan_INCLUDE_DIRS} )
    include_directories(${Vulkan_INCLUDE_DIRS})
endif ()

set(Target niagara)

#
file(GLOB HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/*.h)

set(HEADERS
        VulkanTexture.h
        VulkanUtils.h
        VulkanMesh.h
        Device.h
        VulkanShader.h
        VulkanResourceManager.h
        VulkanGraphicsPipelineBuilder.h
        VulkanDescriptorSetLayoutBuilder.h
        VulkanRenderPassBuilder.h
        VulkanPipelineLayoutBuilder.h
        VulkanCubemapRender.h
        VulkanSwapChain.h
        VulkanImGuiRender.h
        VulkanTexture2DRender.h
        Macro.h
        )

set(SOURCES
        VulkanTexture.cpp
        VulkanUtils.cpp
        VulkanMesh.cpp
        Device.cpp
        VulkanShader.cpp
        VulkanResourceManager.cpp
        VulkanGraphicsPipelineBuilder.cpp
        VulkanDescriptorSetLayoutBuilder.cpp
        VulkanImGuiRender.cpp
        VulkanTexture2DRender.cpp
        VulkanRenderPassBuilder.cpp
        VulkanPipelineLayoutBuilder.cpp
        VulkanCubemapRender.cpp
        VulkanSwapChain.cpp

        VulkanImGuiRender.cpp
        ../extern/imgui/imgui.cpp
        ../extern/imgui/imgui_draw.cpp
        ../extern/imgui/imgui_widgets.cpp
        ../extern/imgui/imgui_tables.cpp
        ../extern/imgui/imgui_demo.cpp
        ../extern/imgui/backends/imgui_impl_glfw.cpp
        ../extern/imgui/backends/imgui_impl_vulkan.cpp
        )

add_library(${Target} SHARED ${SOURCES})