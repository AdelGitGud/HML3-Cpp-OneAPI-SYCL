cmake_minimum_required (VERSION 3.23.0)
cmake_policy(SET CMP0069 NEW)

project(MLE)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/WorkingDir CACHE PATH "Single Directory for Libraries.")

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/WorkingDir CACHE PATH "Single Directory for Executables.")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set( IS_64_BIT 1 )
endif ()


