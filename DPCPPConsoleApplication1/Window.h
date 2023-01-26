#pragma once
#include <string>
#include <vector>

#include "Platform.h"

class Window {
public:
    Window(class RenderManager* renderer, const std::wstring& name, const uint32_t& width, const uint32_t& height);
    ~Window();

    void Close();
    bool Update();

private:
    void InitOsWindow();
    void DeInitOsWindow();

    void UpdateOsWindow();
    void InitOsSurface();

    void InitSurface();
    void DeInitSurface();

    void InitSwapchain();
    void DeInitSwapchain();

    void InitSwapchainImages();
    void DeInitSwapchainImages();

private:
    struct Members {
        class RenderManager* renderer = nullptr;

        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkSurfaceFormatKHR surfaceFormat = {};
        VkSurfaceCapabilitiesKHR surfaceCapabilities = {};

        VkSwapchainKHR swapchain = VK_NULL_HANDLE;

        std::vector<VkImage> swapchainImageList;
        std::vector<VkImageView> swapchainImageViewList;

        std::wstring name;

        uint32_t surfaceWidth = 640;
        uint32_t surfaceHeight = 480;
        uint32_t swapchainImageCount = 2;

        bool shouldWindowRun = true;

#if VK_USE_PLATFORM_WIN32_KHR
        HINSTANCE win32Instance = nullptr;
        HWND win32Window = nullptr;
        std::wstring win32ClassName;
        static uint32_t win32ClassIdCounter;
#elif VK_USE_PLATFORM_XCB_KHR
        xcb_connection_t* xcbConnection = nullptr;
        xcb_screen_t* xcbScreen = nullptr;
        xcb_window_t xcbWindow = 0;
        xcb_intern_atom_reply_t* xcbAtomWindowReply = nullptr;
#endif
    }m;
};
