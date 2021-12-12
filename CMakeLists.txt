cmake_minimum_required(VERSION 3.12)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    message(STATUS "Find Vulkan")
    include_directories(${VULKAN_INCLUDE_DIRS})
endif ()

if(CMAKE_HOST_WIN32)
    message(STATUS "This is Windows Platform")
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR GLFW_EXPOSE_NATIVE_WIN32)
elseif(CMAKE_HOST_UNIX)
    message(STATUS "This is Unix Platform")
    add_compile_options()
elseif(CMAKE_HOST_APPLE)

endif()


set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(THIRD_PARTY_ROOT ${CMAKE_CURRENT_LIST_DIR}/extern)
message(STATUS ${THIRD_PARTY_ROOT})
set(ShaderC_LIBRARIES ${CMAKE_CURRENT_LIST_DIR}/extern/shaderc/lib)
set(Vulkan_LIBRARIES ${CMAKE_CURRENT_LIST_DIR}/extern/vulkan/lib)

if(MSVC)
    #MSVC Setting
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

endif()

#设置debug dll输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
#lib output
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
#可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(extern/volk)
add_subdirectory(extern/glfw )
add_subdirectory(extern/assimp)
add_subdirectory(extern/tinyobjloader)
add_subdirectory(src)
