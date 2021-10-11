cmake_minimum_required(VERSION 3.12)
project(GameEngine )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(../extern/vulkan)
#get_filename_component(ABSOLUTE_PATH ${CMAKE_CURRENT_LIST_DIR} ABSOLUTE)
#set(Vulkan_LIBRARIES ${ABSOLUTE_PATH}/ )
set(Vulkan_LIB_PATH ${CMAKE_CURRENT_LIST_DIR}/extern/vulkan/lib)
message(STATUS ${Vulkan_LIB})
#message(STATUS ${ABSOLUTE_PATH} )

if(CMAKE_HOST_WIN32)
    message(STATUS "This is Windows Platform")
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR GLFW_EXPOSE_NATIVE_WIN32)
elseif(CMAKE_HOST_UNIX)
    message(STATUS "This is Unix Platform")
elseif(CMAKE_HOST_APPLE)
    message(STATUS "This is Apple Platform")
endif()

add_subdirectory(extern/glfw)
add_subdirectory(extern/glm)
add_subdirectory(extern/assimp)
add_subdirectory(src)
