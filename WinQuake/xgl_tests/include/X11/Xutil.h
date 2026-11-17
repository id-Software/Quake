/* Handwritten host compile contract. Not native Xlib declarations/ABI. */
#ifndef XGL_TEST_XUTIL_H
#define XGL_TEST_XUTIL_H
#include "Xlib.h"
typedef struct {
    Visual *visual;
    VisualID visualid;
    int screen, depth, class;
    unsigned long red_mask, green_mask, blue_mask;
    int colormap_size, bits_per_rgb;
} XVisualInfo;
typedef struct {
    long flags;
    int x, y, width, height, min_width, min_height, max_width, max_height;
} XSizeHints;
typedef struct { int compose_ptr, chars_matched; } XComposeStatus;
enum { VisualIDMask = 8, VisualScreenMask = 1, VisualClassMask = 2, VisualDepthMask = 4,
       PPosition = 4, PSize = 8, PMinSize = 16, PMaxSize = 32 };
XVisualInfo *XGetVisualInfo(Display *, long, XVisualInfo *, int *);
int XMatchVisualInfo(Display *, int, int, int, XVisualInfo *);
void XSetWMNormalHints(Display *, Window, XSizeHints *);
int XLookupString(XKeyEvent *, char *, int, KeySym *, XComposeStatus *);
#endif
