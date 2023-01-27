#include "RenderManager.h"

#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <vector>

#include "BUILD_OPTIONS.h"
#include "Shared.h"
#include "Common.h"

const char* validationLayerList[] = {
    "VK_LAYER_KHRONOS_validation"
};

const char* instanceExtensionList[] = {
    
#if BUILD_ENABLE_VULKAN_DEBUG
    
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    
#endif // BUILD_ENABLE_VULKAN_DEBUG
    
    VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME,
    VK_KHR_SURFACE_EXTENSION_NAME,
    PLATFORM_SURFACE_EXTENSION_NAME
};

const char* deviceExtensionList[] = {
    
#if BUILD_ENABLE_VULKAN_DEBUG
    
    VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
    
#endif // BUILD_ENABLE_VULKAN_DEBUG
    
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
    VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
    VK_KHR_RAY_QUERY_EXTENSION_NAME,
    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
};

#if BUILD_ENABLE_VULKAN_DEBUG

PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugReportFlagsEXT flagMsg
    , VkDebugReportObjectTypeEXT objectType
    , uint64_t objectSource
    , size_t location
    , int32_t msgCode
    , const char* layerPrefix
    , const char* msg
    , void* userData) {
    std::ostringstream stream;

    stream << "VKDBG: ";
    if (flagMsg & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        stream << "INFO: ";
    }
    if (flagMsg & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        stream << "WARNING: ";
    }
    if (flagMsg & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        stream << "PERFORMANCE: ";
    }
    if (flagMsg & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        stream << "ERROR: ";
    }
    if (flagMsg & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        stream << "DEBUG: ";
    }
    stream << "@[" << layerPrefix << "]: ";
    stream << msg << std::endl;
    std::cout << stream.str();


    if (flagMsg & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        MsgBox(stream.str().c_str());
    }

    std::exit(-1);

    return false;
}
#endif // BUILD_ENABLE_VULKAN_DEBUG

RenderManager::~RenderManager() {
    Shutdown();
}

bool RenderManager::Init() {
    SetupDebug();
    InitInstance();
    InitDebug();
    InitDevice();
    return true;
}

void RenderManager::Shutdown() {
    CloseWindow();
    ShutdownDevice();
    ShutdownDebug();
    ShutdownInstance();
}

Window* RenderManager::OpenWindow(const std::wstring& name, const uint32_t& width, const uint32_t& height) {
    CloseWindow();
    m.window = new Window(this, name, width, height);
    return m.window;
}

void RenderManager::CloseWindow() {
    if (m.window) {
        delete m.window;
        m.window = nullptr;
    }
}

bool RenderManager::Run() {
    if (m.window != nullptr) {
		Draw();
        return m.window->Update();
    }
    return false;
}

void RenderManager::Draw() {

}

void RenderManager::InitInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Client";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.pEngineName = "MLEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 3, 0);

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledLayerCount = LENGTH(validationLayerList);
    instanceCreateInfo.ppEnabledLayerNames = validationLayerList;
	instanceCreateInfo.enabledExtensionCount = LENGTH(instanceExtensionList);
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensionList;
    instanceCreateInfo.pNext = &m.debugCallbackCreateInfo;

    ErrorCheck(vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &m.instance));

}

void RenderManager::ShutdownInstance() {
    vkDestroyInstance(m.instance, VK_NULL_HANDLE);
    m.instance = VK_NULL_HANDLE;
}

void RenderManager::InitDevice() {
    {
        uint32_t gpuCount = 0;
        vkEnumeratePhysicalDevices(m.instance, &gpuCount, VK_NULL_HANDLE);
        std::vector<VkPhysicalDevice> gpuList(gpuCount);
        vkEnumeratePhysicalDevices(m.instance, &gpuCount, gpuList.data());
        m.gpu = gpuList[0];
        vkGetPhysicalDeviceProperties(m.gpu, &m.gpuProps);
        printf("gpu:\n");
        printf("\tgpuCount: %d\n", gpuCount);
        printf("\tname: %s\n", m.gpuProps.deviceName);
        printf("\tapiVersion: %d\n", m.gpuProps.apiVersion);
        printf("\tdriverVersion: %d\n", m.gpuProps.driverVersion);
        printf("\n");
    }

    {
        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m.gpu, &familyCount, VK_NULL_HANDLE);
        std::vector<VkQueueFamilyProperties> familyPropertyList(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m.gpu, &familyCount, familyPropertyList.data());

        bool found = false;
        for (size_t i = 0; i < familyCount; ++i) {
            if (familyPropertyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                found = true;
                m.gfxFamilyIndex = i;
            }
        }
        if (!found) {
            assert(0 && "tfw no gfx queue fam");
            std::exit(-1);
        }
    }

    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, VK_NULL_HANDLE);
        std::vector<VkLayerProperties> layerPropertiesList(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layerPropertiesList.data());
        printf("Instance layers:\n");
        for (size_t i = 0; i < layerCount; ++i) {
            printf("\t%s: %s\n", layerPropertiesList[i].layerName, layerPropertiesList[i].description);
        }
    }

    {
        uint32_t layerCount = 0;
        vkEnumerateDeviceLayerProperties(m.gpu, &layerCount, VK_NULL_HANDLE);
        std::vector<VkLayerProperties> layerPropertiesList(layerCount);
        vkEnumerateDeviceLayerProperties(m.gpu, &layerCount, layerPropertiesList.data());
        printf("Device layers:\n");
        for (size_t i = 0; i < layerCount; ++i) {
            printf("\t%s: %s\n", layerPropertiesList[i].layerName, layerPropertiesList[i].description);
        }
    }

    float queuePriotities[]{ 1.0f };
    VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.queueFamilyIndex = m.gfxFamilyIndex;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = queuePriotities;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    
#if BUILD_ENABLE_VULKAN_DEBUG
    
    deviceCreateInfo.enabledLayerCount = LENGTH(validationLayerList);
	deviceCreateInfo.ppEnabledLayerNames = validationLayerList;
    
#endif // BUILD_ENABLE_VULKAN_DEBUG
    
	deviceCreateInfo.enabledExtensionCount = LENGTH(deviceExtensionList);
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionList;


    ErrorCheck(vkCreateDevice(m.gpu, &deviceCreateInfo, VK_NULL_HANDLE, &m.device));

    vkGetDeviceQueue(m.device, m.gfxFamilyIndex, 0, &m.queue);

}

void RenderManager::ShutdownDevice() {
    vkDestroyDevice(m.device, VK_NULL_HANDLE);
    m.device = VK_NULL_HANDLE;
}

#if BUILD_ENABLE_VULKAN_DEBUG

void RenderManager::SetupDebug() {
    m.debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    m.debugCallbackCreateInfo.pfnCallback = VulkanDebugCallback;
    m.debugCallbackCreateInfo.flags =
        VK_DEBUG_REPORT_WARNING_BIT_EXT
        | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
        | VK_DEBUG_REPORT_ERROR_BIT_EXT
        | 0;
}

void RenderManager::InitDebug() {
    fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m.instance, "vkCreateDebugReportCallbackEXT");
    fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m.instance, "vkDestroyDebugReportCallbackEXT");

    if (fvkDestroyDebugReportCallbackEXT == VK_NULL_HANDLE || fvkCreateDebugReportCallbackEXT == VK_NULL_HANDLE) {
        assert(0 && "tfw no debug functions");
        std::exit(-1);
    }

    fvkCreateDebugReportCallbackEXT(m.instance, &m.debugCallbackCreateInfo, VK_NULL_HANDLE, &m.debugReport);
}

void RenderManager::ShutdownDebug() {
    fvkDestroyDebugReportCallbackEXT(m.instance, m.debugReport, VK_NULL_HANDLE);
    m.debugReport = VK_NULL_HANDLE;
}

#else

void RenderManager::SetupDebug() {};
void RenderManager::InitDebug() {};
void RenderManager::ShutdownDebug() {};

#endif // BUILD_ENABLE_VULKAN_DEBUG
