cmake_minimum_required(VERSION 3.12)
project(GameEngine )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)


#message(STATUS ${ABSOLUTE_PATH} )
#
#一. SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../bin)
#上面的语句能设置可执行文件的输出目录
#
#在Win + VS环境下,会自动在你所设置的目录后面扩展一层 <CONFIG> 目录,所以最终生成的Debug版本程序会在 ${PROJECT_SOURCE_DIR}/../bin/Debug 目录下,Release版本程序会在 ${PROJECT_SOURCE_DIR}/../bin/Release 目录下.
#
#在Linux + GCC环境下,无论是Debug还是Release,生成的可执行程序会直接放在你所设置的目录下,不会有差异.
#
#二. SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../lib)
#
#上面的语句能设置库文件的输出目录
#
#在Win + VS环境下,会自动在你所设置的目录后面扩展一层 <CONFIG> 目录,所以最终生成的Debug版本库会在 ${PROJECT_SOURCE_DIR}/../lib/Debug 目录下,Release版本库会在 ${PROJECT_SOURCE_DIR}/../lib/Release 目录下.
#
#在Linux + GCC环境下,无论是Debug还是Release,生成的库文件会直接放在你所设置的目录下,不会有差异.

#三. set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/../bin)    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/../bin)
#
#上面两条语句分别设置了Debug版本和Release版本可执行文件的输出目录,
#
#一旦设置上面的属性,在任何环境下生成的可执行文件都将直接放在你所设置的目录.
#
#四. set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/../lib)    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/../lib)
#
#上面两条语句分别设置了Debug版本和Release版本库文件的输出目录,
#
#一旦设置上面的属性,在任何环境下生成的库文件都将直接放在你所设置的目录.
#五. set(CMAKE_DEBUG_POSTFIX "_d")    set(CMAKE_RELEASE_POSTFIX "_r")
#
#上面两条语句分别设置了Debug版本和Release版本下库文件的后缀名.

if(CMAKE_HOST_WIN32)
    message(STATUS "This is Windows Platform")
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR GLFW_EXPOSE_NATIVE_WIN32)
elseif(CMAKE_HOST_UNIX)
    message(STATUS "This is Unix Platform")
elseif(CMAKE_HOST_APPLE)
    message(STATUS "This is Apple Platform")
endif()



set(ShaderC_LIBRARIES ${CMAKE_CURRENT_LIST_DIR}/extern/shaderc/lib)
message(STATUS ${ShaderC_LIBRARIES})

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
add_subdirectory(extern/glm )
add_subdirectory(extern/assimp)

add_subdirectory(src)
