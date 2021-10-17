# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindShaderC
----------

.. versionadded:: 3.7

Find Vulkan, which is a low-overhead, cross-platform 3D graphics
and computing API.

IMPORTED Targets
^^^^^^^^^^^^^^^^

This module defines :prop_tgt:`IMPORTED` target ``ShaderC::ShaderC``, if
Vulkan has been found.


Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables::

  ShaderC_FOUND          - "True" if Vulkan was found
  ShaderC_INCLUDE_DIRS   - include directories for ShaderC
  ShaderC_LIBRARIES      - link against this library to use ShaderC

The module will also define three cache variables::

  ShaderC_INCLUDE_DIR        - the ShaderC include directory
  ShaderC_LIBRARY            - the path to the Vulkan library

Hints
^^^^^

#]=======================================================================]

if(WIN32)
  find_path(ShaderC_INCLUDE_DIR
    NAMES shaderc/shaderc.h
    HINTS
      "$ENV{VULKAN_SDK}/Include"
    )

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(ShaderC_LIBRARY
      NAMES shaderc_combined
      HINTS
        "$ENV{VULKAN_SDK}/Lib"
        "$ENV{VULKAN_SDK}/Bin"
      )
    message(STATUS "FindShaderC.cmake:${ShaderC_LIBRARY}")
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    find_library(ShaderC_LIBRARY
      NAMES shaderc_combined
      HINTS
        "$ENV{VULKAN_SDK}/Lib32"
        "$ENV{VULKAN_SDK}/Bin32"
      )
  endif()
else()
  find_path(ShaderC_INCLUDE_DIR
    NAMES shaderc/shaderc.h
    HINTS "$ENV{VULKAN_SDK}/include")
  find_library(ShaderC_LIBRARY
    NAMES shaderc
    HINTS "$ENV{VULKAN_SDK}/lib")
endif()

message(STATUS  ${ShaderC_LIBRARY})
set(ShaderC_LIBRARIES ${ShaderC_LIBRARY})
set(Shader_INCLUDE_DIRS ${ShaderC_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ShaderC
  DEFAULT_MSG
  ShaderC_LIBRARY ShaderC_INCLUDE_DIR)

mark_as_advanced(ShaderC_INCLUDE_DIR ShaderC_LIBRARY )

if(ShaderC_FOUND AND NOT TARGET ShaderC::ShaderC)
  add_library(ShaderC::ShaderC UNKNOWN IMPORTED)
  set_target_properties(ShaderC::ShaderC PROPERTIES
    IMPORTED_LOCATION "${ShaderC_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${ShaderC_INCLUDE_DIRS}")
endif()

