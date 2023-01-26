#include "Window.h"

#include <assert.h>
#include <iostream>
#include <cstdio>
#include <cstring>

#include "BUILD_OPTIONS.h"
#include "Platform.h"
#include "RenderManager.h"
#include "Shared.h"

#if VK_USE_PLATFORM_XCB_KHR

void Window::InitOsWindow() {
	const xcb_setup_t* setup = nullptr;
	xcb_screen_iterator_t iter;
	int screen = 0;

	xcbConnection = xcb_connect(nullptr, &screen);
	if (xcbConnection == nullptr) {
		std::cout << "Cannot find a compatible Vulkan ICD.\n";
		exit(-1);
	}

	setup = xcb_get_setup(xcbConnection);
	iter = xcb_setup_roots_iterator(setup);
	while (screen-- > 0) {
		xcb_screen_next(&iter);
	}
	xcbScreen = iter.data;

	VkRect2D dimensions = { { 0, 0 }, { m.surfaceWidth, m.surfaceHeight } };

	assert(dimensions.extent.width > 0);
	assert(dimensions.extent.height > 0);

	uint32_t valueMask, valueList[32];

	xcbWindow = xcb_generate_id(xcbConnection);

	valueMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	valueList[0] = xcbScreen->black_pixel;
	valueList[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE;

	xcb_create_window(xcbConnection, XCB_COPY_FROM_PARENT, xcbWindow,
		xcbScreen->root, dimensions.offset.x, dimensions.offset.y,
		dimensions.extent.width, dimensions.extent.height, 0,
		XCB_WINDOW_CLASS_INPUT_OUTPUT, xcbScreen->root_visual,
		valueMask, valueList);

	xcb_intern_atom_cookie_t cookie = xcb_intern_atom(xcbConnection, 1, 12, "WM_PROTOCOLS");
	xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(xcbConnection, cookie, 0);

	xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(xcbConnection, 0, 16, "WM_DELETE_WINDOW");
	xcbAtomWindowReply = xcb_intern_atom_reply(xcbConnection, cookie2, 0);

	xcb_change_property(xcbConnection, XCB_PROP_MODE_REPLACE, xcbWindow,
		(*reply).atom, 4, 32, 1,
		&(*xcbAtomWindowReply).atom);
	free(reply);

	xcb_change_property(xcbConnection,
		XCB_PROP_MODE_REPLACE,
		xcbWindow,
		XCB_ATOM_Wm.name,
		XCB_ATOM_STRING,
		8,
		strlen(m.name.c_str()),
		m.name.c_str());

	xcb_map_window(xcbConnection, xcbWindow);

	const uint32_t coords[] = { 100, 100 };
	xcb_configure_window(xcbConnection, xcbWindow, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
	xcb_flush(xcbConnection);

	xcb_generic_event_t* e;
	while ((e = xcb_wait_for_event(xcbConnection))) {
		if ((e->response_type & ~0x80) == XCB_EXPOSE)
			break;
	}
}

void Window::DeInitOsWindow() {
	xcb_destroy_window(xcbConnection, xcbWindow);
	xcb_disconnect(xcbConnection);
	xcbWindow = 0;
	xcbConnection = nullptr;
}

void Window::UpdateOsWindow() {
	xcb_generic_event_t* event = xcb_poll_for_event(xcbConnection);
	if (!event)
		return;

	switch (event->response_type & ~0x80) {
	case XCB_CLIENT_MESSAGE:
		if (((xcb_client_message_event_t*)event)->data.data32[0] == xcbAtomWindowReply->atom) {
			Close();
		}
		break;
	default:
		break;
	}
	free(event);
}

void Window::InitOsSurface() {
	VkXcbSurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	createInfo.connection = xcbConnection;
	createInfo.window = xcbWindow;
	ErrorCheck(vkCreateXcbSurfaceKHR(m.renderer->GetInstance(), &createInfo, nullptr, &m.surface));
}

#endif