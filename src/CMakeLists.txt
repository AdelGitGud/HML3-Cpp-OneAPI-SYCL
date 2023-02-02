cmake_minimum_required(VERSION 3.23.0)
cmake_policy(SET CMP0069 NEW)

find_package(IntelDPCPP REQUIRED)
find_package(tbb REQUIRED)
find_package(dal REQUIRED)
find_package(IntelSYCL REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)
find_package(imgui REQUIRED)

# Ensure the projects are built as 64-bit only
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Define the source files for the client
set(SRC_CLIENT client/*.cpp)

# Define the source and header files for the common library
set(SRC_COMMON common/*.cpp)
set(HDR_COMMON
	common/HOMLData.h
	common/DPData.h
)

# Define the source files for the server
set(SRC_SERVER server/*.cpp)

# Add the common library
add_library(common STATIC ${SRC_COMMON} ${HDR_COMMON})
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common PRIVATE oneapi)

# Add the client executable
add_executable(client ${SRC_CLIENT})
target_include_directories(client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(client PRIVATE common glm vulkan imgui)
set_target_properties(client PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
)

# Add the server executable
add_executable(server ${SRC_SERVER})
target_include_directories(server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(server PRIVATE common)
set_target_properties(server PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
)

# Use the DPCPP compiler for all projects
set_property(TARGET common PROPERTY CXX_COMPILER icx)
set_property(TARGET client PROPERTY CXX_COMPILER icx)
set_property(TARGET server PROPERTY CXX_COMPILER icx)
