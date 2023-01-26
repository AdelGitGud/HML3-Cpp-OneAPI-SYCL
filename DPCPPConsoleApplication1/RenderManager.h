#pragma once
#include "BaseManager.h"
#include "Window.h"

class RenderManager : public BaseManager {
public:
    bool Init() override;
    void Shutdown() override;
    Window* OpenWindow(const std::wstring& name, const uint32_t& width, const uint32_t& height);

    bool Run();

    VkInstance GetInstance() const { return m.instance; };
    VkDevice GetDevice() const { return m.device; };
    VkQueue GetQueue() const { return m.queue; };
    VkPhysicalDevice GetPhysicalDevice() const { return m.gpu; };
    VkPhysicalDeviceProperties GetPhysicalDeviceProperties() const { return m.gpuProps; };
    uint32_t GetGfxFamilyIndex() const { return m.gfxFamilyIndex; };

private:
    void InitInstance();
    void DeInitInstance();

    void InitDevice();
    void DeInitDevice();

    void SetupDebug();
    void InitDebug();
    void DeInitDebug();

private:
    struct Members {
        VkInstance instance = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkQueue queue = VK_NULL_HANDLE;
        VkPhysicalDevice gpu = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties gpuProps = {};

        uint32_t gfxFamilyIndex = 0;

        Window* window = nullptr;

        VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
        VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo = {};
    }m;
};
