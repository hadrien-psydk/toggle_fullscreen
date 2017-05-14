#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <errno.h>
#include <unistd.h>

#define println(f, ...) \
	printf(f "\n", ##__VA_ARGS__)

#define println_err(f, ...) \
	fprintf(stderr, f "\n", ##__VA_ARGS__)

bool force_properties(Display* display, Window win) {
	// Allowed actions
	Atom wm_allowed_actions = XInternAtom(display, "_NET_WM_ALLOWED_ACTIONS", false);
	Atom wm_action_fullscreen = XInternAtom(display, "_NET_WM_ACTION_FULLSCREEN", false);
	XChangeProperty(display, win, wm_allowed_actions, XA_ATOM, 32, PropModeAppend,
		(unsigned char*)&wm_action_fullscreen, 1);

	// Size hints
	XSizeHints* size_hints_ptr = XAllocSizeHints();
    size_hints_ptr->flags = PMinSize;
    size_hints_ptr->min_width = 10;
    size_hints_ptr->min_height = 10;
	XSetWMNormalHints(display, win, size_hints_ptr);
	XFree(size_hints_ptr);
	return true;
}

bool toggle_fullscreen(Display* display, Window win) {
	int xev_action = 1; // 1 for fullscreen, 0 for normal

	Atom wm_state   = XInternAtom(display, "_NET_WM_STATE", false);
	Atom wm_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", false);

	XEvent xev;
	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = win;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = xev_action;
	xev.xclient.data.l[1] = wm_fullscreen;
	xev.xclient.data.l[2] = 0;  // no second property to toggle
	xev.xclient.data.l[3] = 1;  // source indication: application
	xev.xclient.data.l[4] = 0;  // unused

	Window root_win = DefaultRootWindow(display);

	long evmask = SubstructureRedirectMask | SubstructureNotifyMask;
	if( !XSendEvent(display, root_win, 0, evmask, &xev)) {
	    println_err("XSendEvent failed");
	    return false;
	}

	XWindowAttributes root_win_attr;
	XGetWindowAttributes(display, root_win, &root_win_attr);

	int display_width = root_win_attr.width;
	int display_height = root_win_attr.height;
	XMoveResizeWindow(display, win, 0, 0, display_width, display_height);
	XMapRaised(display, win);
	return true;
}

// Stores a vector of Windows
struct WindowVec {
	const Window* ptr;
	int len;

	WindowVec() : ptr(nullptr), len(0) {}
	WindowVec(Window* ptr_arg, int len_arg) : ptr(ptr_arg), len(len_arg) {}
	~WindowVec() {
		if (ptr) {
			XFree(const_cast<Window*>(ptr));
		}
	}
	Window operator[](int i) const {
		return ptr[i];
	}
};

// C string (zero terminated)
struct CString {
	const char* ptr;

	CString() : ptr(nullptr) {}
	CString(const char* ptr_arg) : ptr(ptr_arg) {}
	~CString() {
		if (ptr) {
			free(const_cast<char*>(ptr));
		}
	}
};

CString get_window_name(Display* display, Window win) {
	Atom prop = XInternAtom(display, "WM_NAME", False);
	Atom type;
	int form;
	unsigned long count;
	unsigned long remain;
	unsigned char* list;

	if (XGetWindowProperty(display, win,
		prop, 0, 1024, False, XA_STRING,
		&type, &form, &count, &remain, &list) != Success) {
		println_err("XGetWindowProperty failed");
		return CString();
	}
	return CString(reinterpret_cast<const char*>(list));
}

WindowVec get_windows(Display* display) {
	Atom prop = XInternAtom(display, "_NET_CLIENT_LIST", False);
	Atom type;
	int form;
	unsigned long count;
	unsigned long remain;
	unsigned char* list;

	if (XGetWindowProperty(display, XDefaultRootWindow(display),
		prop, 0, 1024, False, XA_WINDOW,
		&type, &form, &count, &remain, &list) != Success) {
		println_err("XGetWindowProperty failed");
		return WindowVec();
	}
	return WindowVec(reinterpret_cast<Window*>(list), count);
}

struct OpenDisplay {
	Display* display;

	OpenDisplay(Display* display_arg) : display(display_arg) {}
	~OpenDisplay() {
		XCloseDisplay(display);
	}
};

Window get_window_by_name(Display* display, const WindowVec& windows, const char* name) {
	for (int i = 0; i < windows.len; ++i) {
		CString candidate = get_window_name(display, windows[i]);
		if (!candidate.ptr) {
			println_err("get_window_name failed");
			return 1;
		}
		if (strcmp(candidate.ptr, name) == 0) {
			return windows.ptr[i];
		}
	}
	return 0;
}

int main(int argc, char** argv) {
	if (argc <= 1) {
		println("toggle_fullscreen v1 - Hadrien Nilsson");
		println("");
		println("Toggles a X11 window to fullscreen given its name.");
		println("");
		println("Usage:");
		println("   toggle_fullscreen WINDOW_NAME");
		return 0;
	}
	const char* window_name = argv[1];

	Display* display = XOpenDisplay(nullptr);
	if (!display) {
	    println_err("XOpenDisplay failed");
	    return 1;
	}
	OpenDisplay od(display);

	static const char waiting_str[4][2] = {
		"-", "\\", "|", "/"
	};
	int counter = 0;

	for(;;)
	{
		printf("Waiting for %s window... %s\r", window_name, waiting_str[counter]);
		fflush(stdout);

		counter = (counter + 1) & 0x3;

		WindowVec windows = get_windows(display);
		if (!windows.ptr) {
			println_err("get_windows failed");
			return 1;
		}
		Window win = get_window_by_name(display, windows, window_name);
		if (win) {
			// Found
			if (!force_properties(display, win)) {
				println_err("force_properties failed");
				return 1;
			}
			if (!toggle_fullscreen(display, win)) {
				println_err("toggle_fullscreen failed");
				return 1;
			}
			break;
		}
		usleep(500000);
	}
	printf("\ndone\n");
	return 0;	
}
