/* Syntax-only handwritten contract; NOT Xlib and NOT linkable. */
#ifndef XB_CONTRACT_XUTIL_H
#define XB_CONTRACT_XUTIL_H
#include "Xlib.h"
typedef struct {
    Visual *visual;
    VisualID visualid;
    int screen, depth, class;
    unsigned long red_mask, green_mask, blue_mask;
} XVisualInfo;
typedef struct { long flags; int width, height; } XSizeHints;
enum { VisualScreenMask = 1, VisualClassMask = 2, PSize = 4 };
XVisualInfo *XGetVisualInfo(Display *, long, XVisualInfo *, int *);
void XSetWMNormalHints(Display *, Window, XSizeHints *);
#endif
