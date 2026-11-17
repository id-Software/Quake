/* Syntax-only handwritten contract; NOT Xlib and NOT linkable. */
#ifndef XB_CONTRACT_XLIB_H
#define XB_CONTRACT_XLIB_H
#include <stddef.h>
typedef struct xb_contract_display Display;
typedef struct xb_contract_visual Visual;
typedef unsigned long Window;
typedef unsigned long Colormap;
typedef unsigned long Atom;
typedef unsigned long VisualID;
typedef unsigned long KeySym;
typedef int Bool;
typedef struct { int type; } XKeyEvent;
typedef union {
    int type;
    struct { int type; union { long l[5]; } data; } xclient;
    struct { int type; int state; } xvisibility;
    struct { int type; int width, height; } xconfigure;
    XKeyEvent xkey;
} XEvent;
typedef struct { int width, height, map_state; } XWindowAttributes;
typedef struct {
    Colormap colormap;
    unsigned long background_pixel, border_pixel;
    int backing_store;
    long event_mask;
} XSetWindowAttributes;
enum {
    False = 0, True = 1, TrueColor = 4, AllocNone, NotUseful,
    ExposureMask, StructureNotifyMask, KeyPressMask, VisibilityChangeMask,
    InputOutput, CWColormap, CWBackPixel, CWBorderPixel, CWBackingStore,
    CWEventMask, ClientMessage, DestroyNotify, ConfigureNotify, Expose,
    VisibilityNotify, VisibilityUnobscured, KeyPress, IsViewable
};
Display *XOpenDisplay(const char *);
int XCloseDisplay(Display *);
int DefaultScreen(Display *);
Window RootWindow(Display *, int);
int ConnectionNumber(Display *);
char *DisplayString(Display *);
Colormap XCreateColormap(Display *, Window, Visual *, int);
Window XCreateWindow(Display *, Window, int, int, unsigned int, unsigned int,
                     unsigned int, int, unsigned int, Visual *, unsigned long,
                     XSetWindowAttributes *);
int XDestroyWindow(Display *, Window);
int XFreeColormap(Display *, Colormap);
int XFree(void *);
int XSync(Display *, Bool);
int XPending(Display *);
int XNextEvent(Display *, XEvent *);
int XGetWindowAttributes(Display *, Window, XWindowAttributes *);
int XResizeWindow(Display *, Window, unsigned int, unsigned int);
int XMapRaised(Display *, Window);
int XStoreName(Display *, Window, const char *);
Atom XInternAtom(Display *, const char *, Bool);
int XSetWMProtocols(Display *, Window, Atom *, int);
KeySym XLookupKeysym(XKeyEvent *, int);
#endif
