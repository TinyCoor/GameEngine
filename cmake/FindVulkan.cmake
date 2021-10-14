if(WIN32)
    find_path(Vulkan_INCLUDE_DIR
        NAMES vulkan/vulkan.h
        PATHS $ENV{VULKAN_SDK}/Include)
    if(CMAKE_SIZEOF_VOID EQUAL 8)
        find_library(Vulkan_LIBRARY
        NAMES vulkan-1
        PATHS
            $ENV{VULKAN_SDK}/Bin
            $ENV{VULKAN_SDK}/Lib)

    elseif(CMAKE_SIZEOF_VOID EQUAL 8)
         find_library(Vulkan_LIBRARY
               NAMES vulkan-1
               PATHS
                   $ENV{VULKAN_SDK}/Bin32
                   $ENV{VULKAN_SDK}/Lib32
                   )
    endif()
elseif(APPLE)


else()
    find_path(Vulkan_INCLUDE_DIR
        NAMES vulkan/vulkan.h
        PATHS $ENV{VULKAN_SDK}/include)
    find_library(Vulkan_LIBRARY
                       NAMES vulkan
                       PATHS
                           $ENV{VULKAN_SDK}/lib
                       )
endif()

set(Vulkan_LIBRARIES ${Vulkan_LIBRARY})
set(Vulkan_INCLIDE_DIRS ${Vulkan_INCLUDE_DIE}

include(FindPackageHandleStandargs)
find_package_handle_standard_args(
Vulkan
DEFAULT_MSG
Vulkan_LIBRARY
Vulkan_INCLUDE_DIR
)
mark_as_advanced(Vulkan_LIBRARY Vulkan_INCLUDE_DIR )


if(Vulkan_Found AND NOT TARGET Vulkan::Vulkan)
    add_library(Vulkan::Vulkan UNKNOWN IMPORTED)
    set_target_properties(Vulkan::Vulkan PROPERTIES
    IMPORTED_LOCATION ${Vulkan_LIBRARIES}
    INTERFACE_INCLUDE_DIRECTORIES ${Vulkan_INCLIDE_DIRS}
    )
endif()

