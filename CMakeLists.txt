cmake_minimum_required(VERSION 3.28)

project(OneAPIHOML
    VERSION 0.1.0
    DESCRIPTION "Hands-On Machine Learning in C++20 with oneAPI"
    LANGUAGES CXX
)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------

option(ONEAPP_ENABLE_WARNINGS "Enable strict warnings" ON)
option(ONEAPP_ENABLE_WERROR "Treat warnings as errors" OFF)
option(ONEAPP_ENABLE_LTO "Enable link time optimization" OFF)
option(ONEAPP_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ONEAPP_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ONEAPP_BUILD_CLIENT "Build client application" ON)
option(ONEAPP_BUILD_SERVER "Build server application" ON)

# ------------------------------------------------------------------------------
# Global language settings
# ------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ------------------------------------------------------------------------------
# Shared project options target
# ------------------------------------------------------------------------------

add_library(oneapp_project_options INTERFACE)
add_library(oneapp::project_options ALIAS oneapp_project_options)

target_compile_features(oneapp_project_options INTERFACE cxx_std_20)

if(ONEAPP_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        set_property(TARGET oneapp_project_options PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "LTO requested but not supported: ${ipo_error}")
    endif()
endif()

# ------------------------------------------------------------------------------
# Shared warnings target
# ------------------------------------------------------------------------------

add_library(oneapp_warnings INTERFACE)
add_library(oneapp::warnings ALIAS oneapp_warnings)

if(ONEAPP_ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(oneapp_warnings INTERFACE /W4 /permissive-)
        if(ONEAPP_ENABLE_WERROR)
            target_compile_options(oneapp_warnings INTERFACE /WX)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|IntelLLVM")
        target_compile_options(oneapp_warnings INTERFACE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wconversion
            -Wsign-conversion
            -Wdouble-promotion
            -Wformat=2
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Woverloaded-virtual
        )
        if(ONEAPP_ENABLE_WERROR)
            target_compile_options(oneapp_warnings INTERFACE -Werror)
        endif()
    endif()
endif()

# ------------------------------------------------------------------------------
# Sanitizers
# ------------------------------------------------------------------------------

if(ONEAPP_ENABLE_ASAN OR ONEAPP_ENABLE_UBSAN)
    add_library(oneapp_sanitizers INTERFACE)
    add_library(oneapp::sanitizers ALIAS oneapp_sanitizers)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(oneapp_sanitizers INTERFACE
            $<$<BOOL:${ONEAPP_ENABLE_ASAN}>:-fsanitize=address>
            $<$<BOOL:${ONEAPP_ENABLE_UBSAN}>:-fsanitize=undefined>
            -fno-omit-frame-pointer
        )
        target_link_options(oneapp_sanitizers INTERFACE
            $<$<BOOL:${ONEAPP_ENABLE_ASAN}>:-fsanitize=address>
            $<$<BOOL:${ONEAPP_ENABLE_UBSAN}>:-fsanitize=undefined>
        )
    else()
        message(WARNING "Sanitizers requested, but only GNU/Clang are configured here.")
    endif()
endif()

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

# ---- oneAPI / DPC++ / SYCL / DAL ---------------------------------------------------
#
#   cmake -S . -B build \
#     -DCMAKE_CXX_COMPILER=icpx
#

list(APPEND CMAKE_PREFIX_PATH
    /opt/intel/oneapi/compiler/latest/lib/cmake
    /opt/intel/oneapi/dal/latest/lib/cmake
    /opt/intel/oneapi/tbb/latest/lib/cmake
    /opt/intel/oneapi/mkl/latest/lib/cmake
)

set(ONEDAL_INTERFACE yes)
set(ONEDAL_USE_DPCPP yes)
set(ONEDAL_LINK dynamic)

find_package(oneDAL CONFIG REQUIRED)

message(STATUS "oneDAL include dirs: ${oneDAL_INCLUDE_DIRS}")
message(STATUS "oneDAL libs: ${DAL_LIBRARIES}")

find_package(Threads REQUIRED)

# ---- Vulkan ------------------------------------------------------------------

if(ONEAPP_BUILD_CLIENT)
    find_package(Vulkan REQUIRED)
    find_package(glm REQUIRED)
endif()

# ---- Dear ImGui ---------------------------------------------------------------

if(ONEAPP_BUILD_CLIENT)
    if(EXISTS "${CMAKE_SOURCE_DIR}/external/imgui/CMakeLists.txt")
        add_subdirectory(external/imgui)
        set(ONEAPP_HAS_IMGUI ON)
    else()
        message(WARNING "Dear ImGui not found in external/imgui. Client build may fail unless you add your own imgui target.")
        set(ONEAPP_HAS_IMGUI OFF)
    endif()
endif()

# ------------------------------------------------------------------------------
# Subdirectories
# ------------------------------------------------------------------------------

add_subdirectory(src/common)

if(ONEAPP_BUILD_CLIENT)
    add_subdirectory(src/client)
endif()

if(ONEAPP_BUILD_SERVER)
    add_subdirectory(src/server)
endif()

# ------------------------------------------------------------------------------
# Install/export
# ------------------------------------------------------------------------------

