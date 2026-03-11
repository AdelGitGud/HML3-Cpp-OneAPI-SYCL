add_executable(oneapp_client)

add_executable(oneapp::client ALIAS oneapp_client)

target_sources(oneapp_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        Main.cpp
        RenderManager.cpp
        UserInputManager.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            RenderManager.h
            UserInputManager.h
)

target_include_directories(oneapp_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(oneapp_client
    PRIVATE
        oneapp::common
        oneapp::project_options
        oneapp::warnings
        Vulkan::Vulkan
        glm::glm
)

if(TARGET oneapp::sanitizers)
    target_link_libraries(oneapp_client PRIVATE oneapp::sanitizers)
endif()

if(TARGET imgui)
    target_link_libraries(oneapp_client PRIVATE imgui)
    target_compile_definitions(oneapp_client PRIVATE ONEAPP_HAS_IMGUI=1)
else()
    message(WARNING "No imgui target found; building client without Dear ImGui linkage.")
    target_compile_definitions(oneapp_client PRIVATE ONEAPP_HAS_IMGUI=0)
endif()

target_compile_definitions(oneapp_client
    PRIVATE
        ONEAPP_USE_VULKAN=1
        ONEAPP_USE_VK_KHR_RAY_QUERY=1
)

