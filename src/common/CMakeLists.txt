add_library(oneapp_common)

add_library(oneapp::common ALIAS oneapp_common)

target_sources(oneapp_common
    PRIVATE
        BaseManager.cpp
        ComputeManager.cpp
        LogManager.cpp
        OneAPIManager.cpp
        OneAPP.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            BaseManager.h
            ComputeManager.h
            DPData.h
            HOMLData.h
            LogManager.h
            OneAPIManager.h
            OneAPP.h
)

target_include_directories(oneapp_common
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/oneapp/common>
        ${oneDAL_INCLUDE_DIRS}
)

target_link_libraries(oneapp_common
    PRIVATE
        oneapp::project_options
        oneapp::warnings
    PUBLIC
        Threads::Threads
        ${DAL_LIBRARIES}
)

if(TARGET oneapp::sanitizers)
    target_link_libraries(oneapp_common PRIVATE oneapp::sanitizers)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
    target_compile_options(oneapp_common PUBLIC -fsycl)
    target_link_options(oneapp_common PRIVATE -fsycl)
endif()

target_compile_definitions(oneapp_common
    PUBLIC
        ONEAPP_HAS_ONEAPI=1
)

set_target_properties(oneapp_common PROPERTIES
    OUTPUT_NAME oneapp_common
)

