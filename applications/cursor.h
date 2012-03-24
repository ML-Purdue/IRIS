#ifndef CURSOR_H
#define CURSOR_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static Display *dpy;
static Window root_window;

int init_cursor();
void get_cursor(int* x, int* y);
void set_cursor(int x, int y);
void click_mouse (int button);

#endif
