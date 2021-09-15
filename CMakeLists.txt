cmake_minimum_required(VERSION 3.17)
project(GameEngine)

set(CMAKE_CXX_STANDARD 14)

if(CMAKE_HOST_WIN32)
    message(STATUS "This is Windows Platform")
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR GLFW_EXPOSE_NATIVE_WIN32)
elseif(CMAKE_HOST_UNIX)
    message(STATUS "This is Unix Platform")
elseif(CMAKE_HOST_APPLE)
    message(STATUS "This is Apple Platform")
endif()

function(PRINT_ARGN)
    foreach(arg IN LISTS ARGN)

        message(STATUS “arg =${arg}”)
    endforeach()
#    if(GTEST_FOUND)
#        _target_link_libraries(${ARGN})
#    endif()
endfunction()

# in case Git is not available, we default to "unknown"
set(GIT_HASH "unknown")
# find Git and if available set GIT_HASH variable
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%h
            OUTPUT_VARIABLE GIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            WORKING_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

message(STATUS "Git hash is ${GIT_HASH}")

add_subdirectory(extern/glfw)
add_subdirectory(src)
