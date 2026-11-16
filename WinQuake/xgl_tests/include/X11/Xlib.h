/* Handwritten host compile contract. Not Xlib, not its ABI, never native use. */
#ifndef XGL_TEST_XLIB_H
#define XGL_TEST_XLIB_H
#include <stddef.h>
typedef struct xgl_test_display Display;
typedef struct xgl_test_visual Visual;
typedef unsigned long XID;
typedef XID Window, Drawable, Pixmap, Cursor, Colormap, Atom, VisualID, KeySym, Time;
typedef int Bool;
typedef struct { int type; Display *display; Window window; unsigned int state, keycode; int x, y; Time time; } XKeyEvent;
typedef struct { int type; int x, y; unsigned int state, button; } XButtonEvent;
typedef struct { int type; int x, y; unsigned int state; } XMotionEvent;
typedef union {
    int type;
    XKeyEvent xkey;
    XButtonEvent xbutton;
    XMotionEvent xmotion;
    struct { int type; union { long l[5]; } data; } xclient;
    struct { int type; int state; } xvisibility;
    struct { int type; int x, y, width, height; } xconfigure;
    struct { int type; int mode, detail; } xfocus;
} XEvent;
typedef struct { int width, height, map_state; } XWindowAttributes;
typedef struct {
    Colormap colormap;
    unsigned long background_pixel, border_pixel;
    int backing_store, override_redirect;
    long event_mask;
    Cursor cursor;
} XSetWindowAttributes;
typedef struct { unsigned long pixel; unsigned short red, green, blue; char flags, pad; } XColor;
enum {
    False = 0, True = 1, None = 0, TrueColor = 4, AllocNone = 0,
    NotUseful = 0, InputOutput = 1, CurrentTime = 0, RevertToParent = 2,
    GrabModeAsync = 1, GrabSuccess = 0, IsViewable = 2,
    KeyPress = 2, KeyRelease = 3, ButtonPress = 4, ButtonRelease = 5,
    MotionNotify = 6, EnterNotify = 7, LeaveNotify = 8, FocusIn = 9,
    FocusOut = 10, Expose = 12, VisibilityNotify = 15, DestroyNotify = 17,
    MapNotify = 19, ConfigureNotify = 22, ClientMessage = 33, VisibilityUnobscured = 0
};
#define KeyPressMask (1L << 0)
#define KeyReleaseMask (1L << 1)
#define ButtonPressMask (1L << 2)
#define ButtonReleaseMask (1L << 3)
#define EnterWindowMask (1L << 4)
#define LeaveWindowMask (1L << 5)
#define PointerMotionMask (1L << 6)
#define ButtonMotionMask (1L << 13)
#define ExposureMask (1L << 15)
#define VisibilityChangeMask (1L << 16)
#define StructureNotifyMask (1L << 17)
#define FocusChangeMask (1L << 21)
#define CWBackPixel (1L << 1)
#define CWBorderPixel (1L << 3)
#define CWBackingStore (1L << 6)
#define CWOverrideRedirect (1L << 9)
#define CWEventMask (1L << 11)
#define CWColormap (1L << 13)
#define CWCursor (1L << 14)
Display *XOpenDisplay(const char *);
int XCloseDisplay(Display *);
int DefaultScreen(Display *);
Window RootWindow(Display *, int);
int DefaultDepth(Display *, int);
Visual *DefaultVisual(Display *, int);
unsigned long BlackPixel(Display *, int);
unsigned long WhitePixel(Display *, int);
int DisplayWidth(Display *, int);
int DisplayHeight(Display *, int);
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
int XFlush(Display *);
int XPending(Display *);
int XNextEvent(Display *, XEvent *);
int XPeekEvent(Display *, XEvent *);
int XWindowEvent(Display *, Window, long, XEvent *);
int XGetWindowAttributes(Display *, Window, XWindowAttributes *);
int XResizeWindow(Display *, Window, unsigned int, unsigned int);
int XMapRaised(Display *, Window);
int XMapWindow(Display *, Window);
int XStoreName(Display *, Window, const char *);
Atom XInternAtom(Display *, const char *, Bool);
int XSetWMProtocols(Display *, Window, Atom *, int);
KeySym XLookupKeysym(XKeyEvent *, int);
int XSelectInput(Display *, Window, long);
int XWarpPointer(Display *, Window, Window, int, int, unsigned int, unsigned int, int, int);
int XGrabPointer(Display *, Window, Bool, unsigned int, int, int, Window, Cursor, Time);
int XUngrabPointer(Display *, Time);
int XGrabKeyboard(Display *, Window, Bool, int, int, Time);
int XUngrabKeyboard(Display *, Time);
int XAutoRepeatOn(Display *);
int XAutoRepeatOff(Display *);
int XDefineCursor(Display *, Window, Cursor);
int XUndefineCursor(Display *, Window);
Pixmap XCreateBitmapFromData(Display *, Drawable, const char *, unsigned int, unsigned int);
Cursor XCreatePixmapCursor(Display *, Pixmap, Pixmap, XColor *, XColor *, unsigned int, unsigned int);
int XFreePixmap(Display *, Pixmap);
int XFreeCursor(Display *, Cursor);
#endif
