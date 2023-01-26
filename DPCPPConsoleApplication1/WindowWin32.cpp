#include "Window.h"

#include <assert.h>
#include <iostream>

#include "BUILD_OPTIONS.h"
#include "Platform.h"
#include "RenderManager.h"
#include "Shared.h"

#if VK_USE_PLATFORM_WIN32_KHR
LRESULT CALLBACK WindowsEventHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	Window* window = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

	switch (uMsg) {
	case WM_CLOSE:
		window->Close();
		return 0;
	case WM_SIZE:
		// TODO: Handle resize
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

uint32_t	Window::Members::win32ClassIdCounter = 0;

void Window::InitOsWindow() {
	WNDCLASSEX winCLass{};
	assert(m.surfaceWidth > 0);
	assert(m.surfaceHeight > 0);

	m.win32Instance = GetModuleHandle(nullptr);
	m.win32ClassName = m.name + L"_" + std::to_wstring(m.win32ClassIdCounter);
	m.win32ClassIdCounter++;

	winCLass.cbSize = sizeof(WNDCLASSEX);
	winCLass.style = CS_HREDRAW | CS_VREDRAW;
	winCLass.lpfnWndProc = WindowsEventHandler;
	winCLass.cbClsExtra = 0;
	winCLass.cbWndExtra = 0;
	winCLass.hInstance = m.win32Instance;
	winCLass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winCLass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winCLass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winCLass.lpszMenuName = NULL;
	winCLass.lpszClassName = m.win32ClassName.c_str();
	winCLass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	if (!RegisterClassEx(&winCLass)) {
		assert(0 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}

	DWORD exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	RECT wr = { 0, 0, LONG(m.surfaceWidth), LONG(m.surfaceHeight) };
	AdjustWindowRectEx(&wr, style, FALSE, exStyle);
	m.win32Window = CreateWindowEx(0,
		m.win32ClassName.c_str(),		// class name
		m.name.c_str(),					// app name
		style,							// window style
		CW_USEDEFAULT, CW_USEDEFAULT,	// x/y coords
		wr.right - wr.left,				// width
		wr.bottom - wr.top,				// height
		NULL,							// handle to parent
		NULL,							// handle to menu
		m.win32Instance,				// hInstance
		NULL);							// no extra parameters
	if (!m.win32Window) {
		assert(1 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}
	SetWindowLongPtr(m.win32Window, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(m.win32Window, SW_SHOW);
	SetForegroundWindow(m.win32Window);
	SetFocus(m.win32Window);
}

void Window::DeInitOsWindow() {
	DestroyWindow(m.win32Window);
	UnregisterClass(m.win32ClassName.c_str(), m.win32Instance);
}

void Window::UpdateOsWindow() {
	MSG msg;
	if (PeekMessage(&msg, m.win32Window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Window::InitOsSurface() {
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = m.win32Instance;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.hwnd = m.win32Window;
	ErrorCheck(vkCreateWin32SurfaceKHR(m.renderer->GetInstance(), &createInfo, nullptr, &m.surface));

}
#endif
