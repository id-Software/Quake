/* Native XGL platform boundary; GPL-2.0-or-later. */
#ifndef XGL_BACKEND_H
#define XGL_BACKEND_H
#include <X11/Xlib.h>
void XGLB_Init(Display *display, Window window, int width, int height);
void XGLB_Shutdown(void);
void XGLB_Resize(int width, int height);
extern unsigned char xgl_palette[768];
#endif
