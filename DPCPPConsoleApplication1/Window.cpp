#include "Window.h"

#include <assert.h>

#include "RenderManager.h"
#include "Shared.h"

Window::Window(class RenderManager* renderer,const std::wstring& name, const uint32_t& width, const uint32_t& height) {
    m.renderer = renderer;
    m.surfaceWidth = width;
    m.surfaceHeight = height;
    m.name = name;

    InitOsWindow();
    InitSurface();
    InitSwapchain();
    InitSwapchainImages();
}

Window::~Window() {
    ShutdownSwapchainImages();
    ShutdownSwapchain();
    DeInitSurface();
    DeInitOsWindow();
}

void Window::Close() {
    m.shouldWindowRun = false;
}

bool Window::Update() {
    UpdateOsWindow();
    return m.shouldWindowRun;
}

void Window::InitSurface() {
    InitOsSurface();

    VkPhysicalDevice physicalDevice = m.renderer->GetPhysicalDevice();

    VkBool32 isWsiSupported = false;
    ErrorCheck(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, m.renderer->GetGfxFamilyIndex(), m.surface, &isWsiSupported));
    if (!isWsiSupported) {
        assert(0 && "WSI not supported :(");
        std::exit(-1);
    }

    ErrorCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m.surface, &m.surfaceCapabilities));

    if (m.surfaceCapabilities.currentExtent.width < UINT32_MAX) {
        m.surfaceWidth = m.surfaceCapabilities.currentExtent.width;
        m.surfaceHeight = m.surfaceCapabilities.currentExtent.height;
    }

    {
        uint32_t formatCount = 0;
        ErrorCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m.surface, &formatCount, nullptr));
        if (formatCount == 0) {
            assert(0 && "Surface formats missing");
            std::exit(-1);
        }
        std::vector<VkSurfaceFormatKHR> formatList(formatCount);
        ErrorCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m.surface, &formatCount, formatList.data()));

        if (formatList[0].format == VK_FORMAT_UNDEFINED) {
            m.surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
            m.surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        }
        else {
            m.surfaceFormat = formatList[0];
        }
    }
}

void Window::DeInitSurface() {
    vkDestroySurfaceKHR(m.renderer->GetInstance(), m.surface, nullptr);
}

void Window::InitSwapchain() {
    if (m.swapchainImageCount < m.surfaceCapabilities.minImageCount + 1)
        m.swapchainImageCount = m.surfaceCapabilities.minImageCount + 1;
    if (m.swapchainImageCount > 0 && m.swapchainImageCount > m.surfaceCapabilities.maxImageCount)
        m.swapchainImageCount = m.surfaceCapabilities.maxImageCount;

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    {
        uint32_t presentModeCount = 0;
        ErrorCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(m.renderer->GetPhysicalDevice(), m.surface, &presentModeCount, nullptr));
        std::vector<VkPresentModeKHR> presentModeList(presentModeCount);
        ErrorCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(m.renderer->GetPhysicalDevice(), m.surface, &presentModeCount, presentModeList.data()));

        for (auto m : presentModeList) {
            if (m == VK_PRESENT_MODE_MAILBOX_KHR)
                presentMode = m;
        }
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = m.surface;
    swapchainCreateInfo.minImageCount = m.swapchainImageCount;
    swapchainCreateInfo.imageFormat = m.surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = m.surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent.width = m.surfaceWidth;
    swapchainCreateInfo.imageExtent.height = m.surfaceHeight;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    ErrorCheck(vkCreateSwapchainKHR(m.renderer->GetDevice(), &swapchainCreateInfo, nullptr, &m.swapchain));

    ErrorCheck(vkGetSwapchainImagesKHR(m.renderer->GetDevice(), m.swapchain, &m.swapchainImageCount, nullptr));
}

void Window::ShutdownSwapchain() {
    vkDestroySwapchainKHR(m.renderer->GetDevice(), m.swapchain, nullptr);
}

void Window::InitSwapchainImages() {
    m.swapchainImageList.resize(m.swapchainImageCount);
    m.swapchainImageViewList.resize(m.swapchainImageCount);

    ErrorCheck(vkGetSwapchainImagesKHR(m.renderer->GetDevice(), m.swapchain, &m.swapchainImageCount, m.swapchainImageList.data()));

    for (uint32_t i = 0; i < m.swapchainImageCount; i++) {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = m.swapchainImageList[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = m.surfaceFormat.format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        ErrorCheck(vkCreateImageView(m.renderer->GetDevice(), &imageViewCreateInfo, nullptr, &m.swapchainImageViewList[i]));
    }
}

void Window::ShutdownSwapchainImages() {
    for (auto view : m.swapchainImageViewList) {
        vkDestroyImageView(m.renderer->GetDevice(), view, nullptr);
    }
}
