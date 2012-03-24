#include "cursor.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int
init_cursor ()
{
    dpy = XOpenDisplay(0);
    root_window = XRootWindow(dpy, 0);
    XSelectInput(dpy, root_window, KeyReleaseMask);
}

void
set_cursor (int x, int y)
{
    XWarpPointer(dpy, None, root_window, 0, 0, 0, 0, x, y);

    XFlush(dpy);
}

void
get_cursor (int* x, int* y) {
  Window w;
  int i;
  XQueryPointer(dpy, root_window, &w, &w, x, y, &i, &i, (unsigned int *)&i);
}

void click_mouse (int button)
{
    XEvent event;

    memset(&event, 0x00, sizeof(event));

    event.type = ButtonPress;
    event.xbutton.button = button;
    event.xbutton.same_screen = True;

    XQueryPointer(dpy, root_window, &event.xbutton.root,
            &event.xbutton.window, &event.xbutton.x_root,
            &event.xbutton.y_root, &event.xbutton.x,
            &event.xbutton.y, &event.xbutton.state);

    event.xbutton.subwindow = event.xbutton.window;

    while(event.xbutton.subwindow)
    {
        event.xbutton.window = event.xbutton.subwindow;

        XQueryPointer(dpy, event.xbutton.window,
                &event.xbutton.root, &event.xbutton.subwindow,
                &event.xbutton.x_root, &event.xbutton.y_root,
                &event.xbutton.x, &event.xbutton.y,
                &event.xbutton.state);
    }

    if(XSendEvent(dpy, PointerWindow, True, 0xfff, &event) == 0){
        fprintf(stderr, "Error\n");
    }

    XFlush(dpy);

    usleep(100000);

    event.type = ButtonRelease;
    event.xbutton.state = 0x100;

    if(XSendEvent(dpy, PointerWindow, True, 0xfff, &event) == 0) {
        fprintf(stderr, "Error\n");
    }

    XFlush(dpy);

    XCloseDisplay(dpy);
}

