// 
// Original code by Adam Pierce - http://www.doctort.org/adam/
// http://www.doctort.org/adam/nerd-notes/x11-fake-keypress-event.html
//

//
// find_window() -function contributed by Markus Lindqvist
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/Xevie.h>

// evdev input
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define VERSION "v0.3-SVN"
#define VENTWIN "Ventrilo"

// define key to send to Ventrilo, default is A
#define KEYCODE XK_A

XKeyEvent createKeyEvent(Display *display, Window win,
			Window winRoot, int press,
			int keycode, int modifiers)
{
	XKeyEvent event;

	event.display     = display;
	event.window      = win;
	event.root        = winRoot;
	event.subwindow   = None;
	event.time        = CurrentTime;
	event.x           = 1;
	event.y           = 1;
	event.x_root      = 1;
	event.y_root      = 1;
	event.same_screen = True;
	event.keycode     = XKeysymToKeycode(display, keycode);
	event.state       = modifiers;
	event.send_event  = 1;
	event.serial      = 0;
	if(press)
		event.type = KeyPress;
	else
		event.type = KeyRelease;

	return event;
}

Window *find_window(Display *display, Window start, const char *name)
{
	Window *children = 0;
	Window root;
	Window parent;
	int i;
	unsigned int childrenCount = 0;
	XQueryTree(display, start, &root, &parent, &children, &childrenCount);
	    
	char *window_name = 0;
	for(i=0; i<childrenCount; ++i) {
		if(XFetchName(display, children[i], &window_name) == 1) {
			//printf("value: %s\n", window_name);
			if(strcmp(name, window_name) == 0) {
				XFree(window_name);
				return &children[i];
			}
			XFree(window_name);
		}
	}

	for(i=0; i<childrenCount; ++i) {
		Window *window = find_window(display, children[i], name);
		if(window != 0) {
			XFree(children);
			return window;
		}
	}

	if(children != 0)
		XFree(children);

	return 0;
}

int main(int argc, char **argv)
{
	// xlib
	Display *display;
	Window winRoot;
	Window *winFocus = 0;

	// check parameters
	if (argc < 2) {
		printf("usage: %s <listen key>\n", argv[0]);
		return 1;
	}

	printf("Ventrilo Control %s for Linux by Purkka Productions\n", VERSION);
	printf("=====================================================\n");
	printf("WARNING: This program reads DIRECTLY your keyboard!\n");
	printf("         You need correct permissions to read the\n");
	printf("         event device.\n");

	// evdev
	XEvent xev;
	XKeyEvent event;
	
	display = XOpenDisplay(0);
	if(display == NULL) {
		printf("Error: Could not open display!\n");
		return 1;
	}
	
	if(!XevieStart(display)) {
		printf("XevieStart(disp) failure!\n");
		return 1;
	}
	
	XevieSelectInput(display, KeyPressMask | KeyReleaseMask);
	//I have never figured out how to get this working.
	//XkbSetDetectableAutoRepeat(display, True, NULL);
	
	winRoot = XDefaultRootWindow(display);
	winFocus = find_window(display, winRoot, VENTWIN);

	if(!winFocus) {
		printf("Could not find Ventrilo window. Is Ventrilo running?\n");
		return 1;
	}

	printf ("Window selected. Please test pressing the key to talk before going to play.\n");
	printf ("\nUse CTRL-C to quit.\n");

	// main loop after selecting window
	while(1)
	{
		//read(fd, &ev, sizeof(struct input_event));
		XNextEvent(display, &xev);
		
		//unsigned int iKeyCode = xev.xkey.keycode;
		//unsigned int iKeyCode = XLookupKeysym(&xev.xkey, xev.xkey.state);
		unsigned int iKeyCode = XLookupKeysym(&xev.xkey, 0);
		//unsigned int iKeyState = xev.xkey.state;
		//unsigned long iKeyTime = xev.xkey.time;
		int iKeyType = xev.type;
		
		//printf("C++: MsgLoop - Key fingerd(%i)\n", iKeyCode);
		if (atoi(argv[1]) == iKeyCode) {
			switch (iKeyType) {
				case KeyPress:
					printf("C++: MsgLoop - Key pressed(%i)\n", iKeyCode);
					event = createKeyEvent(display, *winFocus, winRoot, 1, iKeyCode, 0);
					XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *) &event);
					XFlush(display);
				break;
				
				case KeyRelease:
					//This code is if XkbSetDetectableAutoRepeat, but it seems to fail regardless with Xevie.
					
					/* Linux has an issue with running the keysends....  If you can get the keyrepetes to stop with xevie more power to you.
					unsigned int iQueue = XEventsQueued(xev.xkey.display, QueuedAfterReading);
					if (iQueue) {
						XPeekEvent (xev.xkey.display, &xev_next);
					}
					
					if (!iQueue || xev_next.type != KeyPress && xev_next.xkey.keycode != xev.xkey.keycode && xev_next.xkey.time - xev.xkey.time <= 1) {
						printf("C++: MsgLoop - Key released(%i)\n", iKeyCode);
					}
					*/
					printf("C++: MsgLoop - Key released(%i)\n", iKeyCode);
					event = createKeyEvent(display, *winFocus, winRoot, 0, iKeyCode, 0);
					XSendEvent(event.display, event.window, True, KeyReleaseMask, (XEvent *) &event);
					XFlush(display);
				break;
			}
		}
		
		XevieSendEvent(display, &xev, XEVIE_UNMODIFIED);
	}

	return 0;
}
