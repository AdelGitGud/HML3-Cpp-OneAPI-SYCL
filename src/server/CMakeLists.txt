add_executable(oneapp_server)

add_executable(oneapp::server ALIAS oneapp_server)

target_sources(oneapp_server
    PRIVATE
        Main.cpp
)

target_link_libraries(oneapp_server
    PRIVATE
        oneapp::common
        oneapp::project_options
        oneapp::warnings
)

if(TARGET oneapp::sanitizers)
    target_link_libraries(oneapp_server PRIVATE oneapp::sanitizers)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
    target_compile_options(oneapp_server PRIVATE -fsycl)
    target_link_options(oneapp_server PRIVATE -fsycl)
endif()
