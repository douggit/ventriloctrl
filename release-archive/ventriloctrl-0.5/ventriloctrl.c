/*******************************************************************/
/* This version of ventriloctl by Caleb Gray (caleb@calebgray.com) */
/*******************************************************************/

/* Console Output */
#include <stdio.h>

/* Input Detection */
#include <fcntl.h>
#include <linux/input.h>

/* X11 */
#include <X11/Xlib.h>
#include <X11/keysym.h>

/* Configuration */
#define SIMULATEKEY XK_A    // Simulate Key Press
#define VENTRILO "Ventrilo" // Ventrilo Window Name

/* Global Variables */
Display *display;
Window window;
Window *windowVentrilo;

/* Functions */
XKeyEvent create_key_event(Display *display, Window window, Window windowRoot, int press, int keycode, int modifiers)
{
	XKeyEvent event;
	event.display = display;
	event.window = window;
	event.root = windowRoot;
	event.subwindow = None;
	event.time = CurrentTime;
	event.x = 1;
	event.y = 1;
	event.x_root = 1;
	event.y_root = 1;
	event.same_screen = True;
	event.keycode = keycode;
	event.state = modifiers;
	event.send_event = 1;
	event.serial = 0;
	if (press) { event.type = KeyPress; } else { event.type = KeyRelease; }
	return event;
}

int ventrilo_still_running()
{
	char *windowName = VENTRILO;
	return XFetchName(display, *windowVentrilo, &windowName) != BadWindow;
}

Window *find_window(Display *display, Window parentWindow, const char *windowName)
{
	// Variables
	int i;
	
	// Get the "Window" Tree
	Window root;
	Window parent;
	Window *children = 0;
	unsigned int childrenCount = 0;
	XQueryTree(display, parentWindow, &root, &parent, &children, &childrenCount);
	
	// Crawl the "Window" Tree
	char *window_name = 0;
	Window *window = 0;
	for (i = 0; i < childrenCount; ++i) {
		// Look For "windowName"
		if (XFetchName(display, children[i], &window_name) == 1) {
			if (strcmp(windowName, window_name) == 0) {
				XFree(window_name);
				return &children[i];
			}
			XFree(window_name);
		}
		
		// Crawl the Child and Look For "windowName"
		window = find_window(display, children[i], windowName);
		if (window != 0) {
			XFree(children);
			return window;
		}
	}
	
	// Free the Children! lol
	if (children != 0) XFree(children);
	
	// Return
	return 0;
}

int ventriloctl(int argc, char **argv)
{
	// Usage
	printf("\n");
	
	// Initialize "Display"
	display = XOpenDisplay(0);
	if (display == NULL) {
		printf("Error: Could not open display!\n");
		return 1;
	}
	
	// Initialize Root "Window"
	window = XDefaultRootWindow(display);
	if (!window) {
		printf("Error: Could not grab root window!\n");
		return 2;
	}
	
	// Initialize Ventrilo "Window"
	windowVentrilo = find_window(display, window, VENTRILO);
	if (!windowVentrilo) {
		printf("Error: Could not find Ventrilo window!\n");
		return 3;
	}
	
	// Convert Key String to Number
	unsigned int key = atoi(argv[2]);
	unsigned int simulatekey = XKeysymToKeycode(display, SIMULATEKEY);
	
	// Open the Input
	int device = open(argv[1], O_RDONLY);
	
	// Loop
	struct input_event ev;
	XKeyEvent event;
	while (ventrilo_still_running()) {
		// Read from the Input
		read(device, &ev, sizeof(struct input_event));
		
		// Check Input
		if (ev.type == 1 && ev.code == key) {
			// Simulate Key Press/Release
			event = create_key_event(display, *windowVentrilo, window, ev.value, simulatekey, 0);
			if (ev.value == 1) {
				XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *) &event);
			} else {
				XSendEvent(event.display, event.window, True, KeyReleaseMask, (XEvent *) &event);
			}
			XFlush(display);
		}
	}
	
	// Return
	return 0;
}

int findkey(int argc, char **argv)
{
	// Usage
	printf("\n");
	printf("Press the key you would like to use as\n");
	printf("your Ventrilo Push-To-Talk key now,\n");
	printf("then execute the command listed.\n");
	printf("Press Ctrl+C when finished.\n");
	printf("\n");
	
	// Open the Input
	int device = open(argv[1], O_RDONLY);
	
	// Loop
	struct input_event ev;
	while (1) {
		// Read from the Input
		read(device, &ev, sizeof(struct input_event));
		
		// Print the Command
		if (ev.type == 1 && ev.value == 1) printf("%s %s %i\n", argv[0], argv[1], ev.code);
	}
	
	// Return
	return 0;
}

/* Main Loop */
int main(int argc, char **argv)
{
	// Programs
	if(argc == 2) return findkey(argc, argv);	// Find Key
	if(argc == 3) return ventriloctl(argc, argv);	// Ventrilo Ctrl
	
	// Usage
	printf("\n");
	printf("Usage: %s <device> [key]\n", argv[0]);
	printf("\n");
	printf("Running this program without 'key' specified\n");
	printf("will run the 'FindKey' sub-program.\n");
	printf("\n");
	
	// Return
	return 0;
}
