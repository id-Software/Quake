/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

/* XGL window and input driver. Based on the Solaris X11 input conventions;
 * the software framebuffer and OpenGL drivers remain separate targets. */
#include "quakedef.h"
#include "xgl_backend.h"
#include "xgl_smp.h"
#include <X11/Xutil.h>
#include <X11/keysym.h>

viddef_t vid;
unsigned short d_8to16table[256];
unsigned d_8to24table[256];
byte xgl_palette[768];
void (*vid_menudrawfn)(void);
void (*vid_menukeyfn)(int key);

static Display *xgl_display;
static Window xgl_window;
static Colormap xgl_colormap;
static Atom xgl_delete;
static Cursor xgl_cursor;
static int xgl_width, xgl_height, xgl_focused, xgl_grabbed, xgl_mouse;
static int xgl_last_x, xgl_last_y;
static float xgl_mouse_x, xgl_mouse_y, xgl_old_x, xgl_old_y;
static cvar_t xgl_mouse_filter = {"m_filter", "0", true, false, 0, NULL};
extern qboolean keydown[256];
extern void M_Menu_Options_f(void);
extern void M_Print(int x, int y, char *text);
extern void M_PrintWhite(int x, int y, char *text);
static const int xgl_modes[][2] = {{320,240}, {640,480}, {800,600}, {1024,768}};
static int xgl_menu_mode;

static void XGL_ReleaseKeys(int first, int last)
{
    int key;
    for (key = first; key <= last; ++key)
        if (keydown[key]) Key_Event(key, false);
}

static int XGL_Key(XKeyEvent *event)
{
    KeySym symbol;
    int key;
    /* Physical key identity must survive a Shift release before key-up. */
    symbol = XLookupKeysym(event, 0);
    switch (symbol) {
    case XK_Page_Up: case XK_KP_Page_Up: return K_PGUP;
    case XK_Page_Down: case XK_KP_Page_Down: return K_PGDN;
    case XK_Home: case XK_KP_Home: return K_HOME;
    case XK_End: case XK_KP_End: return K_END;
    case XK_Left: case XK_KP_Left: return K_LEFTARROW;
    case XK_Right: case XK_KP_Right: return K_RIGHTARROW;
    case XK_Up: case XK_KP_Up: return K_UPARROW;
    case XK_Down: case XK_KP_Down: return K_DOWNARROW;
    case XK_Escape: return K_ESCAPE;
    case XK_Return: case XK_KP_Enter: return K_ENTER;
    case XK_Tab: return K_TAB;
    case XK_BackSpace: case XK_Delete: case XK_KP_Delete: return K_BACKSPACE;
    case XK_Insert: case XK_KP_Insert: return K_INS;
    case XK_Pause: return K_PAUSE;
    case XK_Shift_L: case XK_Shift_R: return K_SHIFT;
    case XK_Control_L: case XK_Control_R: return K_CTRL;
    case XK_Alt_L: case XK_Alt_R: case XK_Meta_L: case XK_Meta_R: return K_ALT;
    case XK_F1: return K_F1; case XK_F2: return K_F2;
    case XK_F3: return K_F3; case XK_F4: return K_F4;
    case XK_F5: return K_F5; case XK_F6: return K_F6;
    case XK_F7: return K_F7; case XK_F8: return K_F8;
    case XK_F9: return K_F9; case XK_F10: return K_F10;
    case XK_F11: return K_F11; case XK_F12: return K_F12;
    case XK_F27: return K_HOME; case XK_F29: return K_PGUP;
    case XK_F33: return K_END; case XK_F35: return K_PGDN;
    default:
        key = symbol >= 32 && symbol < 127 ? (int)symbol : 0;
        /* Shift processing belongs to keys.c, including console text. */
        if (key >= 'A' && key <= 'Z') key += 'a' - 'A';
        return key;
    }
}

void IN_ClearStates(void)
{
    xgl_mouse_x = xgl_mouse_y = xgl_old_x = xgl_old_y = 0;
}

static void XGL_GrabMouse(void)
{
    int wanted;
    if (!xgl_display || !xgl_window) return;
    wanted = xgl_mouse && xgl_focused && key_dest == key_game && !cl.paused;
    if (wanted == xgl_grabbed) return;
    IN_ClearStates();
    if (wanted) {
        if (XGrabPointer(xgl_display, xgl_window, True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, xgl_window, xgl_cursor,
                CurrentTime) != GrabSuccess) return;
        xgl_grabbed = 1;
        xgl_last_x = xgl_width / 2;
        xgl_last_y = xgl_height / 2;
        XWarpPointer(xgl_display, None, xgl_window, 0, 0, 0, 0,
                     xgl_last_x, xgl_last_y);
    } else {
        XGL_ReleaseKeys(K_MOUSE1, K_MOUSE3);
        XUngrabPointer(xgl_display, CurrentTime);
        xgl_grabbed = 0;
    }
    XFlush(xgl_display);
}

void Sys_SendKeyEvents(void)
{
    XEvent event, next;
    int key, button, width, height;
    if (!xgl_display) return;
    while (XPending(xgl_display)) {
        XNextEvent(xgl_display, &event);
        switch (event.type) {
        case KeyRelease:
            /* X11 autorepeat sends a release immediately followed by a press. */
            if (XPending(xgl_display)) {
                XPeekEvent(xgl_display, &next);
                if (next.type == KeyPress && next.xkey.time == event.xkey.time &&
                    next.xkey.keycode == event.xkey.keycode) break;
            }
            key = XGL_Key(&event.xkey);
            if (key) Key_Event(key, false);
            break;
        case KeyPress:
            key = XGL_Key(&event.xkey);
            if (key) Key_Event(key, true);
            break;
        case ButtonPress: case ButtonRelease:
            button = event.xbutton.button;
            key = button == 1 ? K_MOUSE1 : button == 2 ? K_MOUSE3 :
                  button == 3 ? K_MOUSE2 : button == 4 ? K_MWHEELUP :
                  button == 5 ? K_MWHEELDOWN : 0;
            if (key) Key_Event(key, event.type == ButtonPress);
            break;
        case MotionNotify:
            if (xgl_grabbed) {
                xgl_mouse_x += event.xmotion.x - xgl_last_x;
                xgl_mouse_y += event.xmotion.y - xgl_last_y;
                xgl_last_x = event.xmotion.x;
                xgl_last_y = event.xmotion.y;
            }
            break;
        case FocusIn:
            xgl_focused = 1;
            break;
        case FocusOut:
            xgl_focused = 0;
            XGL_ReleaseKeys(0, 255);
            Key_ClearStates();
            IN_ClearStates();
            break;
        case ConfigureNotify:
            width = event.xconfigure.width;
            height = event.xconfigure.height;
            /* Window-manager size hints are advisory. Keep the engine's
             * console and view arithmetic within its supported dimensions. */
            if (width < 320 || height < 200 || width > 4096 || height > 4096) {
                width = width < 320 ? 320 : (width > 4096 ? 4096 : width);
                height = height < 200 ? 200 : (height > 4096 ? 4096 : height);
                XResizeWindow(xgl_display, xgl_window, width, height);
                XSync(xgl_display, False);
            }
            if (width != xgl_width || height != xgl_height) {
                if (width < 1 || height < 1) break;
                xgl_width = width; xgl_height = height;
                vid.width = vid.conwidth = width;
                vid.height = vid.conheight = height;
                vid.aspect = ((float)height / width) * (320.0f / 240.0f);
                vid.recalc_refdef = 1;
                XGLB_Resize(width, height);
                Sbar_Changed();
                Con_CheckResize();
                IN_ClearStates();
            }
            break;
        case Expose:
            Sbar_Changed();
            break;
        case ClientMessage:
            if ((Atom)event.xclient.data.l[0] == xgl_delete)
                Cbuf_AddText("quit\n");
            break;
        case DestroyNotify:
            xgl_window = 0;
            Sys_Quit();
            return;
        default: break;
        }
    }
    XGL_GrabMouse();
    if (xgl_grabbed && (xgl_last_x != xgl_width / 2 || xgl_last_y != xgl_height / 2)) {
        xgl_last_x = xgl_width / 2; xgl_last_y = xgl_height / 2;
        XWarpPointer(xgl_display, None, xgl_window, 0, 0, 0, 0, xgl_last_x, xgl_last_y);
        XFlush(xgl_display);
    }
}

void IN_Init(void)
{
    Cvar_RegisterVariable(&xgl_mouse_filter);
    xgl_mouse = COM_CheckParm("-nomouse") == 0;
    IN_ClearStates();
}

void IN_Shutdown(void)
{
    xgl_mouse = 0;
    XGL_GrabMouse();
    IN_ClearStates();
}

void IN_Commands(void) { XGL_GrabMouse(); }

void IN_Move(usercmd_t *cmd)
{
    float mx, my;
    if (!xgl_grabbed) { IN_ClearStates(); return; }
    mx = xgl_mouse_x; my = xgl_mouse_y;
    if (xgl_mouse_filter.value) {
        mx = (mx + xgl_old_x) * .5f;
        my = (my + xgl_old_y) * .5f;
    }
    xgl_old_x = xgl_mouse_x; xgl_old_y = xgl_mouse_y;
    xgl_mouse_x = xgl_mouse_y = 0;
    mx *= sensitivity.value; my *= sensitivity.value;
    if ((in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1)))
        cmd->sidemove += m_side.value * mx;
    else cl.viewangles[YAW] -= m_yaw.value * mx;
    if (in_mlook.state & 1) V_StopPitchDrift();
    if ((in_mlook.state & 1) && !(in_strafe.state & 1)) {
        cl.viewangles[PITCH] += m_pitch.value * my;
        if (cl.viewangles[PITCH] > 80) cl.viewangles[PITCH] = 80;
        if (cl.viewangles[PITCH] < -70) cl.viewangles[PITCH] = -70;
    } else if ((in_strafe.state & 1) && noclip_anglehack)
        cmd->upmove -= m_forward.value * my;
    else cmd->forwardmove -= m_forward.value * my;
}

void VID_SetPalette(unsigned char *palette)
{
    unsigned i, r, g, b;
    memcpy(xgl_palette, palette, sizeof(xgl_palette));
    for (i = 0; i < 256; ++i) {
        r = palette[i * 3]; g = palette[i * 3 + 1]; b = palette[i * 3 + 2];
        d_8to24table[i] = r | (g << 8) | (b << 16) | 0xff000000U;
        d_8to16table[i] = (unsigned short)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
    }
    d_8to24table[255] &= 0x00ffffffU;
}

void VID_ShiftPalette(unsigned char *palette)
{
    /* TrueColor has no writable palette. XGL_PolyBlend applies the engine's
     * cshifts; keep texture indices based on the original game palette. */
    (void)palette;
}

static int XGL_ArgumentSize(const char *name, int fallback)
{
    int index, value;
    index = COM_CheckParm((char *)name);
    if (!index) return fallback;
    if (index + 1 >= com_argc) Sys_Error("%s needs a value", name);
    value = Q_atoi(com_argv[index + 1]);
    if (value < 200 || value > 4096) Sys_Error("%s must be between 200 and 4096", name);
    return value;
}

static void XGL_VideoMenuDraw(void)
{
    char text[64];
    int i;
    M_PrintWhite(80, 32, "XGL VIDEO MODES");
    for (i = 0; i < 4; ++i) {
        sprintf(text, "%c %d x %d", i == xgl_menu_mode ? '>' : ' ',
                xgl_modes[i][0], xgl_modes[i][1]);
        M_Print(80, 56 + i * 16, text);
    }
    M_PrintWhite(48, 144, "Enter: apply   Escape: back");
}

static void XGL_VideoMenuKey(int key)
{
    if (key == K_ESCAPE) M_Menu_Options_f();
    else if (key == K_UPARROW) xgl_menu_mode = (xgl_menu_mode + 3) % 4;
    else if (key == K_DOWNARROW) xgl_menu_mode = (xgl_menu_mode + 1) % 4;
    else if (key == K_ENTER && xgl_display) {
        XResizeWindow(xgl_display, xgl_window,
                       xgl_modes[xgl_menu_mode][0], xgl_modes[xgl_menu_mode][1]);
        XFlush(xgl_display);
    }
}

void VID_Init(unsigned char *palette)
{
    XVisualInfo visual, requested, *matches;
    XSetWindowAttributes attributes;
    XSizeHints hints;
    XColor color;
    Pixmap bitmap;
    XEvent event;
    char empty = 0;
    char *display_name;
    int screen, index, match_count;
    xgl_width = XGL_ArgumentSize("-width", 320);
    xgl_height = XGL_ArgumentSize("-height", 240);
    index = COM_CheckParm("-winsize");
    if (index) {
        if (index + 2 >= com_argc) Sys_Error("-winsize needs width and height");
        xgl_width = Q_atoi(com_argv[index + 1]);
        xgl_height = Q_atoi(com_argv[index + 2]);
        if (xgl_width < 320 || xgl_width > 4096 || xgl_height < 200 || xgl_height > 4096)
            Sys_Error("XGL window dimensions must be 320..4096 by 200..4096");
    }
    if (xgl_width < 320) Sys_Error("XGL requires at least 320 pixels of width");
    display_name = NULL;
    index = COM_CheckParm("-display");
    if (index && index + 1 < com_argc) display_name = com_argv[index + 1];
    xgl_display = XOpenDisplay(display_name);
    if (!xgl_display) Sys_Error("XGL: cannot open X display");
    screen = DefaultScreen(xgl_display);
    index = COM_CheckParm("-visualid");
    if (index) {
        if (index + 1 >= com_argc) Sys_Error("-visualid needs an X visual ID");
        memset(&requested, 0, sizeof(requested));
        requested.visualid = strtoul(com_argv[index + 1], NULL, 0);
        requested.screen = screen;
        matches = XGetVisualInfo(xgl_display, VisualIDMask | VisualScreenMask, &requested, &match_count);
        if (!matches || match_count < 1) Sys_Error("XGL: requested visual not found");
        visual = matches[0];
        XFree(matches);
        if (visual.depth != 24 || visual.class != TrueColor)
            Sys_Error("XGL: requested visual must be 24-bit TrueColor");
    } else if (!XMatchVisualInfo(xgl_display, screen, 24, TrueColor, &visual))
        Sys_Error("XGL: a 24-bit TrueColor visual is required");
    xgl_colormap = XCreateColormap(xgl_display, RootWindow(xgl_display, screen), visual.visual, AllocNone);
    memset(&attributes, 0, sizeof(attributes));
    attributes.colormap = xgl_colormap;
    attributes.border_pixel = attributes.background_pixel = 0;
    attributes.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask |
        ButtonReleaseMask | PointerMotionMask | FocusChangeMask | ExposureMask | StructureNotifyMask;
    xgl_window = XCreateWindow(xgl_display, RootWindow(xgl_display, screen), 0, 0,
        xgl_width, xgl_height, 0, visual.depth, InputOutput, visual.visual,
        CWColormap | CWBorderPixel | CWBackPixel | CWEventMask, &attributes);
    if (!xgl_window) Sys_Error("XGL: cannot create window");
    XStoreName(xgl_display, xgl_window, "XGLQuake");
    memset(&hints, 0, sizeof(hints));
    hints.flags = PMinSize | PMaxSize;
    hints.min_width = 320; hints.min_height = 200;
    hints.max_width = hints.max_height = 4096;
    XSetWMNormalHints(xgl_display, xgl_window, &hints);
    xgl_delete = XInternAtom(xgl_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(xgl_display, xgl_window, &xgl_delete, 1);
    bitmap = XCreateBitmapFromData(xgl_display, xgl_window, &empty, 1, 1);
    memset(&color, 0, sizeof(color));
    if (bitmap) {
        xgl_cursor = XCreatePixmapCursor(xgl_display, bitmap, bitmap, &color, &color, 0, 0);
        XFreePixmap(xgl_display, bitmap);
    }
    XMapWindow(xgl_display, xgl_window);
    do { XWindowEvent(xgl_display, xgl_window, StructureNotifyMask, &event); }
    while (event.type != MapNotify);
    XSync(xgl_display, False);
    memset(&vid, 0, sizeof(vid));
    vid.width = vid.conwidth = xgl_width;
    vid.height = vid.conheight = xgl_height;
    vid.aspect = ((float)xgl_height / xgl_width) * (320.0f / 240.0f);
    vid.colormap = host_colormap;
    vid.fullbright = 256 - LittleLong(*((int *)vid.colormap + 2048));
    vid.numpages = 2;
    vid.recalc_refdef = 1;
    vid_menudrawfn = XGL_VideoMenuDraw;
    vid_menukeyfn = XGL_VideoMenuKey;
    VID_SetPalette(palette);
    XGLB_Init(xgl_display, xgl_window, xgl_width, xgl_height);
    Con_Printf("XGLQuake: %dx%d TrueColor window; native text/geometry UI\n", xgl_width, xgl_height);
}

void VID_Shutdown(void)
{
    XGL_SMPShutdown();
    IN_Shutdown();
    XGLB_Shutdown();
    XGL_WorldShutdown();
    XGL_EntitiesShutdown();
    XGL_TextureShutdown();
    if (!xgl_display) return;
    if (xgl_cursor) XFreeCursor(xgl_display, xgl_cursor);
    if (xgl_window) XDestroyWindow(xgl_display, xgl_window);
    if (xgl_colormap) XFreeColormap(xgl_display, xgl_colormap);
    XCloseDisplay(xgl_display);
    xgl_display = NULL; xgl_window = 0; xgl_cursor = 0; xgl_colormap = 0;
}

void VID_Update(vrect_t *rects) { (void)rects; }
int VID_SetMode(int mode, unsigned char *palette) { (void)mode; VID_SetPalette(palette); return 1; }
void VID_SetDefaultMode(void) { }
void VID_HandlePause(qboolean pause) { (void)pause; XGL_GrabMouse(); }
void D_BeginDirectRect(int x, int y, byte *bitmap, int width, int height)
{ (void)x; (void)y; (void)bitmap; (void)width; (void)height; Draw_BeginDisc(); }
void D_EndDirectRect(int x, int y, int width, int height)
{ (void)x; (void)y; (void)width; (void)height; Draw_EndDisc(); }
