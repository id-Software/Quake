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
// vid_x.c -- general x video driver

#define _BSD

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/extensions/XShm.h>

#include "quakedef.h"
#include "d_local.h"

cvar_t		m_filter = {"m_filter","0", true};

qboolean        mouse_avail;
int             mouse_buttons=3;
int             mouse_oldbuttonstate;
int             mouse_buttonstate;
float   mouse_x, mouse_y;
float   old_mouse_x, old_mouse_y;
int p_mouse_x;
int p_mouse_y;
qboolean	mouse_grabbed = false; // we grab it when console is up

int		VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes, VGA_planar;
byte	*VGA_pagebase;

// The following X property format is defined in Motif 1.1's
// Xm/MwmUtils.h, but QUAKE should not depend on that header
// file. Note: Motif 1.2 expanded this structure with
// uninteresting fields (to QUAKE) so just stick with the
// smaller Motif 1.1 structure.

#define MWM_HINTS_DECORATIONS   2
typedef struct
{
	long flags;
	long functions;
	long decorations;
	long input_mode;
} MotifWmHints;

#define MAX_COLUMN_SIZE	11

#define MAX_MODEDESCS	(MAX_COLUMN_SIZE*3)

typedef struct
{
    int		modenum;
    int		iscur;
    char	desc[256];
} modedesc_t;

extern void M_Menu_Options_f (void);
extern void M_Print (int cx, int cy, char *str);
extern void M_PrintWhite (int cx, int cy, char *str);
extern void M_DrawCharacter (int cx, int line, int num);
extern void M_DrawTransPic (int x, int y, qpic_t *pic);
extern void M_DrawPic (int x, int y, qpic_t *pic);

extern int sb_updates;
extern int x_root, y_root; // root window relative mouse coords

typedef struct
{
	int input;
	int output;
} keymap_t;

viddef_t vid; // global video state
unsigned short       d_8to16table[256];

int		num_shades=32;

int	d_con_indirect = 0;

int		vid_buffersize;

#define STD_EVENT_MASK \
( KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | \
PointerMotionMask | EnterWindowMask | LeaveWindowMask | VisibilityChangeMask | \
ExposureMask | StructureNotifyMask )

qboolean				x_fullscreen = true;
Display					*x_disp = NULL;
int						x_screen, x_screen_width, x_screen_height;
int				x_center_width, x_center_height;
int						x_std_event_mask = STD_EVENT_MASK;
Window					x_win, x_root_win;
qboolean				mouse_in_window = false;
int				global_dx, global_dy;

static qboolean			doShm;
static Colormap			x_cmap;
static GC				x_gc;
static Visual			*x_vis;
static XVisualInfo		*x_visinfo;
static Atom				aHints = 0;
static Atom				aWMDelete = 0;

static int				x_shmeventtype;
//static XShmSegmentInfo	x_shminfo;

static qboolean			oktodraw = false;

int XShmQueryExtension(Display *);
int XShmGetEventBase(Display *);

int current_framebuffer;
static XImage			*x_framebuffer[2] = { 0, 0 };
static XShmSegmentInfo	x_shminfo[2];

static int verbose=1;

static byte current_palette[768];

typedef unsigned short PIXEL16;
typedef unsigned long PIXEL24;
static PIXEL16 st2d_8to16table[256];
static PIXEL24 st2d_8to24table[256];
static int shiftmask_fl=0;
static long r_shift,g_shift,b_shift;
static unsigned long r_mask,g_mask,b_mask;

void shiftmask_init()
{
    unsigned int x;
    r_mask=x_vis->red_mask;
    g_mask=x_vis->green_mask;
    b_mask=x_vis->blue_mask;
    for(r_shift=-8,x=1;x<r_mask;x=x<<1)r_shift++;
    for(g_shift=-8,x=1;x<g_mask;x=x<<1)g_shift++;
    for(b_shift=-8,x=1;x<b_mask;x=x<<1)b_shift++;
    shiftmask_fl=1;
}

PIXEL16 xlib_rgb16(int r,int g,int b)
{
    PIXEL16 p;
    if(shiftmask_fl==0) shiftmask_init();
    p=0;

    if(r_shift>0) {
        p=(r<<(r_shift))&r_mask;
    } else if(r_shift<0) {
        p=(r>>(-r_shift))&r_mask;
    } else p|=(r&r_mask);

    if(g_shift>0) {
        p|=(g<<(g_shift))&g_mask;
    } else if(g_shift<0) {
        p|=(g>>(-g_shift))&g_mask;
    } else p|=(g&g_mask);

    if(b_shift>0) {
        p|=(b<<(b_shift))&b_mask;
    } else if(b_shift<0) {
        p|=(b>>(-b_shift))&b_mask;
    } else p|=(b&b_mask);

    return p;
}

PIXEL24 xlib_rgb24(int r,int g,int b)
{
    PIXEL24 p;
    if(shiftmask_fl==0) shiftmask_init();
    p=0;

    if(r_shift>0) {
        p=(r<<(r_shift))&r_mask;
    } else if(r_shift<0) {
        p=(r>>(-r_shift))&r_mask;
    } else p|=(r&r_mask);

    if(g_shift>0) {
        p|=(g<<(g_shift))&g_mask;
    } else if(g_shift<0) {
        p|=(g>>(-g_shift))&g_mask;
    } else p|=(g&g_mask);

    if(b_shift>0) {
        p|=(b<<(b_shift))&b_mask;
    } else if(b_shift<0) {
        p|=(b>>(-b_shift))&b_mask;
    } else p|=(b&b_mask);

    return p;
}

void st2_fixup( XImage *framebuf, int x, int y, int width, int height)
{
	int xi,yi;
	unsigned char *src;
	PIXEL16 *dest;
	register int count, n;

	if( (x<0)||(y<0) )return;

	for (yi = y; yi < (y+height); yi++) {
		src = &framebuf->data [yi * framebuf->bytes_per_line];

		// Duff's Device
		count = width;
		n = (count + 7) / 8;
		dest = ((PIXEL16 *)src) + x+width - 1;
		src += x+width - 1;

		switch (count % 8) {
		case 0:	do {	*dest-- = st2d_8to16table[*src--];
		case 7:			*dest-- = st2d_8to16table[*src--];
		case 6:			*dest-- = st2d_8to16table[*src--];
		case 5:			*dest-- = st2d_8to16table[*src--];
		case 4:			*dest-- = st2d_8to16table[*src--];
		case 3:			*dest-- = st2d_8to16table[*src--];
		case 2:			*dest-- = st2d_8to16table[*src--];
		case 1:			*dest-- = st2d_8to16table[*src--];
				} while (--n > 0);
		}

//		for(xi = (x+width-1); xi >= x; xi--) {
//			dest[xi] = st2d_8to16table[src[xi]];
//		}
	}
}

void st3_fixup( XImage *framebuf, int x, int y, int width, int height)
{
	int xi,yi;
	unsigned char *src;
	PIXEL24 *dest;
	register int count, n;

	if( (x<0)||(y<0) )return;

	for (yi = y; yi < (y+height); yi++) {
		src = &framebuf->data [yi * framebuf->bytes_per_line];

		// Duff's Device
		count = width;
		n = (count + 7) / 8;
		dest = ((PIXEL24 *)src) + x+width - 1;
		src += x+width - 1;

		switch (count % 8) {
		case 0:	do {	*dest-- = st2d_8to24table[*src--];
		case 7:			*dest-- = st2d_8to24table[*src--];
		case 6:			*dest-- = st2d_8to24table[*src--];
		case 5:			*dest-- = st2d_8to24table[*src--];
		case 4:			*dest-- = st2d_8to24table[*src--];
		case 3:			*dest-- = st2d_8to24table[*src--];
		case 2:			*dest-- = st2d_8to24table[*src--];
		case 1:			*dest-- = st2d_8to24table[*src--];
				} while (--n > 0);
		}

//		for(xi = (x+width-1); xi >= x; xi--) {
//			dest[xi] = st2d_8to16table[src[xi]];
//		}
	}
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
// direct drawing of the "accessing disk" icon isn't supported under Nextstep
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
// direct drawing of the "accessing disk" icon isn't supported under Nextstep
}


/*
=================
VID_Gamma_f

Keybinding command
=================
*/

byte vid_gamma[256];

void VID_Gamma_f (void)
{

	float	g, f, inf;
	int		i;

	if (Cmd_Argc () == 2)
	{
		g = Q_atof (Cmd_Argv(1));

		for (i=0 ; i<255 ; i++)
		{
			f = pow ((i+1)/256.0, g);
			inf = f*255 + 0.5;
			if (inf < 0)
				inf = 0;
			if (inf > 255)
				inf = 255;
			vid_gamma[i] = inf;
		}

		VID_SetPalette (current_palette);

		vid.recalc_refdef = 1;				// force a surface cache flush
	}

}

// ========================================================================
// Tragic death handler
// ========================================================================

void TragicDeath(int signal_num)
{
	//XAutoRepeatOn(x_disp);
	VID_Shutdown();
	Sys_Error("This death brought to you by the number %d\n", signal_num);
}

// ========================================================================
// makes a null cursor
// ========================================================================

static Cursor CreateNullCursor(Display *display, Window root)
{
    Pixmap cursormask; 
    XGCValues xgc;
    GC gc;
    XColor dummycolour;
    Cursor cursor;

    cursormask = XCreatePixmap(display, root, 1, 1, 1/*depth*/);
    xgc.function = GXclear;
    gc =  XCreateGC(display, cursormask, GCFunction, &xgc);
    XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
    dummycolour.pixel = 0;
    dummycolour.red = 0;
    dummycolour.flags = 04;
    cursor = XCreatePixmapCursor(display, cursormask, cursormask,
          &dummycolour,&dummycolour, 0,0);
    XFreePixmap(display,cursormask);
    XFreeGC(display,gc);
    return cursor;
}

void ResetFrameBuffer(void)
{

	int mem;
	int pwidth;

	if (x_framebuffer[0])
	{
		Z_Free(x_framebuffer[0]->data);
//		Z_Free(d_pzbuffer);
		free(x_framebuffer[0]);
	}

	pwidth = x_visinfo->depth / 8;
	if (pwidth == 3) pwidth = 4;
	mem = ((vid.width*pwidth+3)&~3) * vid.height;

//	d_pzbuffer = (unsigned short *) Z_Malloc(vid.width*vid.height*
//		sizeof(*d_pzbuffer));
	d_pzbuffer = (short *) Hunk_HighAllocName(vid.width*vid.height*
		sizeof(*d_pzbuffer), "zbuff");

	x_framebuffer[0] = XCreateImage(	x_disp,
		x_vis,
		x_visinfo->depth,
		ZPixmap,
		0,
		Z_Malloc(mem),
		vid.width, vid.height,
		32,
		0);

	if (!x_framebuffer[0])
		Sys_Error("VID: XCreateImage failed\n");

}

void ResetSharedFrameBuffers(void)
{

	int size;
	int key;
	int minsize = getpagesize();
	int frm;

//	if (d_pzbuffer)
//		Z_Free(d_pzbuffer);
	d_pzbuffer = Hunk_HighAllocName(vid.width*vid.height*sizeof(*d_pzbuffer),"zbuff");

	for (frm=0 ; frm<2 ; frm++)
	{

	// free up old frame buffer memory

		if (x_framebuffer[frm])
		{
			XShmDetach(x_disp, &x_shminfo[frm]);
			free(x_framebuffer[frm]);
			shmdt(x_shminfo[frm].shmaddr);
		}

	// create the image

		x_framebuffer[frm] = XShmCreateImage(	x_disp,
						x_vis,
						x_visinfo->depth,
						ZPixmap,
						0,
						&x_shminfo[frm],
						vid.width,
						vid.height );

	// grab shared memory

		size = x_framebuffer[frm]->bytes_per_line
			* x_framebuffer[frm]->height;
		if (size < minsize)
			Sys_Error("VID: Window must use at least %d bytes\n", minsize);

		key = random();
		x_shminfo[frm].shmid = shmget((key_t)key, size, IPC_CREAT|0777);
		if (x_shminfo[frm].shmid==-1)
			Sys_Error("VID: Could not get any shared memory\n");

		// attach to the shared memory segment
		x_shminfo[frm].shmaddr =
			(void *) shmat(x_shminfo[frm].shmid, 0, 0);

		printf("VID: shared memory id=%d, addr=0x%x\n", x_shminfo[frm].shmid,
			(int) x_shminfo[frm].shmaddr);

		x_framebuffer[frm]->data = x_shminfo[frm].shmaddr;

	// get the X server to attach to it

		if (!XShmAttach(x_disp, &x_shminfo[frm]))
			Sys_Error("VID: XShmAttach() failed\n");
		XSync(x_disp, 0);
		shmctl(x_shminfo[frm].shmid, IPC_RMID, 0);

	}

}

void VID_MenuDraw( void )
{
    qpic_t		*p;
    char		*ptr;
    int			i, j, column, row, dup;
    char		temp[100];

    p = Draw_CachePic ("gfx/vidmodes.lmp");
    M_DrawPic ( (320-p->width)/2, 4, p);
	M_Print (4*8, 36 + MAX_COLUMN_SIZE * 8 + 8, "Video mode switching unavailable");
	M_Print (9*8, 36 + MAX_COLUMN_SIZE * 8 + 8*6, "Press any key...");
}

void VID_MenuKey( int key ) { M_Menu_Options_f (); }

// Called at startup to set up translation tables, takes 256 8 bit RGB values
// the palette data will go away after the call, so it must be copied off if
// the video driver will need it again

byte	surfcache[1024*1024];

//
// VID_SetWindowTitle - set the window and icon titles
//

void VID_SetWindowTitle( Window win, char *pszName )
{
    XTextProperty	textprop;
    XWMHints		*wmHints;

    // Setup ICCCM properties
    textprop.value = (unsigned char *)pszName;
    textprop.encoding = XA_STRING;
    textprop.format = 8;
    textprop.nitems = strlen(pszName);
    wmHints = XAllocWMHints();
    wmHints->initial_state = NormalState;
    wmHints->flags = StateHint;
    XSetWMProperties( x_disp, win, &textprop, &textprop,
					  // Only put WM_COMMAND property on first window.
					  com_argv, com_argc, NULL, NULL, NULL );
    XFree( wmHints );

    aWMDelete = XInternAtom( x_disp, "WM_DELETE_WINDOW", False );
    XSetWMProtocols( x_disp, win, &aWMDelete, 1 );
}

//
// VID_FullScreen - open the window in full screen mode
//

qboolean VID_FullScreen( Window win )
{
    MotifWmHints    hints;
    XWindowChanges  changes;

    aHints = XInternAtom( x_disp, "_MOTIF_WM_HINTS", 0 );
    if (aHints == None)
    {
		Con_Printf( "Could not intern X atom for _MOTIF_WM_HINTS." );
		return( false );
    }

    hints.flags = MWM_HINTS_DECORATIONS;
    hints.decorations = 0; // Absolutely no decorations.
    XChangeProperty( x_disp, win, aHints, aHints, 32, PropModeReplace, (unsigned char *)&hints, 4 );

    changes.x = 0;
    changes.y = 0;
    changes.width = x_screen_width;
    changes.height = x_screen_height;
    changes.stack_mode = TopIf;
    XConfigureWindow( x_disp, win, CWX | CWY | CWWidth | CWHeight | CWStackMode, &changes);
    return( true );
}

void	VID_Init (unsigned char *palette)
{

	int pnum, i;
	XVisualInfo template;
	int num_visuals;
	int template_mask;

	Cmd_AddCommand ("gamma", VID_Gamma_f);
	for (i=0 ; i<256 ; i++)
		vid_gamma[i] = i;

	vid.width = 320;
	vid.height = 200;
	vid.aspect = 1.0;
	vid.numpages = 2;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	//vid.cbits = VID_CBITS;
	//vid.grades = VID_GRADES;

	srandom(getpid());

	verbose=COM_CheckParm("-verbose");

// open the display
	x_disp = XOpenDisplay(0);
	if (!x_disp)
	{
		if (getenv("DISPLAY"))
			Sys_Error("VID: Could not open display [%s]\n",
				getenv("DISPLAY"));
		else
			Sys_Error("VID: Could not open local display\n");
	}

    x_screen = XDefaultScreen( x_disp );
    x_screen_width = WidthOfScreen( ScreenOfDisplay( x_disp, x_screen ) );
    x_screen_height = HeightOfScreen( ScreenOfDisplay( x_disp, x_screen ) );

	x_center_width  = x_screen_width/2;

	x_center_height = x_screen_height/2;

    Con_Printf( "Using screen %d: %dx%d\n", x_screen, x_screen_width, x_screen_height );

    x_root_win = XRootWindow( x_disp, x_screen );

// catch signals so i can turn on auto-repeat
// we never run full-screen, so no auto-repeat nukage
	if (0)
	{
		struct sigaction sa;
		sigaction(SIGINT, 0, &sa);
		sa.sa_handler = TragicDeath;
		sigaction(SIGINT, &sa, 0);
		sigaction(SIGTERM, &sa, 0);
	}

	//XAutoRepeatOff(x_disp);

// for debugging only
//	XSynchronize(x_disp, True);

// check for command-line window size
	if ((pnum=COM_CheckParm("-winsize")))
	{
		if (pnum >= com_argc-2)
			Sys_Error("VID: -winsize <width> <height>\n");
		vid.width = Q_atoi(com_argv[pnum+1]);
		vid.height = Q_atoi(com_argv[pnum+2]);
		if (!vid.width || !vid.height)
			Sys_Error("VID: Bad window width/height\n");
	}

	template_mask = 0;

// specify a visual id
	if ((pnum=COM_CheckParm("-visualid")))
	{
		if (pnum >= com_argc-1)
			Sys_Error("VID: -visualid <id#>\n");
		template.visualid = Q_atoi(com_argv[pnum+1]);
		template_mask = VisualIDMask;
	}

// If not specified, use default visual
	else
	{
		int screen;
		screen = XDefaultScreen(x_disp);
		template.visualid =
			XVisualIDFromVisual(XDefaultVisual(x_disp, screen));
		template_mask = VisualIDMask;
	}

// pick a visual- warn if more than one was available
	x_visinfo = XGetVisualInfo(x_disp, template_mask, &template, &num_visuals);
	if (num_visuals > 1)
	{
		printf("Found more than one visual id at depth %d:\n", template.depth);
		for (i=0 ; i<num_visuals ; i++)
			printf("	-visualid %d\n", (int)(x_visinfo[i].visualid));
	}
	else if (num_visuals == 0)
	{
		if (template_mask == VisualIDMask)
			Sys_Error("VID: Bad visual id %d\n", template.visualid);
		else
			Sys_Error("VID: No visuals at depth %d\n", template.depth);
	}

	if (verbose)
	{
		printf("Using visualid %d:\n", (int)(x_visinfo->visualid));
		printf("	class %d\n", x_visinfo->class);
		printf("	screen %d\n", x_visinfo->screen);
		printf("	depth %d\n", x_visinfo->depth);
		printf("	red_mask 0x%x\n", (int)(x_visinfo->red_mask));
		printf("	green_mask 0x%x\n", (int)(x_visinfo->green_mask));
		printf("	blue_mask 0x%x\n", (int)(x_visinfo->blue_mask));
		printf("	colormap_size %d\n", x_visinfo->colormap_size);
		printf("	bits_per_rgb %d\n", x_visinfo->bits_per_rgb);
	}

	x_vis = x_visinfo->visual;

// setup attributes for main window
	{
		int attribmask = CWEventMask  | CWColormap | CWBorderPixel;
		XSetWindowAttributes attribs;
		Colormap tmpcmap;

		tmpcmap = XCreateColormap(x_disp, XRootWindow(x_disp,
			x_visinfo->screen), x_vis, AllocNone);

		attribs.event_mask = x_std_event_mask;
		attribs.border_pixel = 0;
		attribs.colormap = tmpcmap;

// create the main window
		x_win = XCreateWindow(	x_disp,
			XRootWindow(x_disp, x_visinfo->screen),
			0, 0,	// x, y
			vid.width, vid.height,
			0, // borderwidth
			x_visinfo->depth,
			InputOutput,
			x_vis,
			attribmask,
			&attribs );

		if (x_visinfo->class != TrueColor)
			XFreeColormap(x_disp, tmpcmap);

	}

	if (x_visinfo->depth == 8)
	{

	// create and upload the palette
		if (x_visinfo->class == PseudoColor)
		{
			x_cmap = XCreateColormap(x_disp, x_win, x_vis, AllocAll);
			VID_SetPalette(palette);
			XSetWindowColormap(x_disp, x_win, x_cmap);
		}

	}

    VID_SetWindowTitle( x_win, "Quake" );

// create the GC
	{
		XGCValues xgcvalues;
		int valuemask = GCGraphicsExposures;
		xgcvalues.graphics_exposures = False;
		x_gc = XCreateGC(x_disp, x_win, valuemask, &xgcvalues );
	}

// map the window
	XMapWindow(x_disp, x_win);

// wait for first exposure event
	{
		XEvent event;
		do
		{
			XNextEvent(x_disp, &event);
			if (event.type == Expose && !event.xexpose.count)
				oktodraw = true;
		} while (!oktodraw);
	}
// now safe to draw

// even if MITSHM is available, make sure it's a local connection
	if (XShmQueryExtension(x_disp))
	{
		char *displayname;
		doShm = true;
		displayname = (char *) getenv("DISPLAY");
		if (displayname)
		{
			char *d = displayname;
			while (*d && (*d != ':')) d++;
			if (*d) *d = 0;
			if (!(!strcasecmp(displayname, "unix") || !*displayname))
				doShm = false;
		}
	}

	if (doShm)
	{
		x_shmeventtype = XShmGetEventBase(x_disp) + ShmCompletion;
		ResetSharedFrameBuffers();
	}
	else
		ResetFrameBuffer();

	current_framebuffer = 0;
	vid.rowbytes = x_framebuffer[0]->bytes_per_line;
	vid.buffer = x_framebuffer[0]->data;
	vid.conbuffer = x_framebuffer[0]->data;
	vid.conrowbytes = vid.rowbytes;
	vid.conwidth = vid.width;
	vid.conheight = vid.height;

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;

	D_InitCaches (surfcache, sizeof(surfcache));
	
//	XSynchronize(x_disp, False);

	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;

}

void VID_ShiftPalette(unsigned char *p)
{
	VID_SetPalette(p);
}

void VID_SetPalette(unsigned char *palette)
{

	int i;
	XColor colors[256];

	for(i=0;i<256;i++) {
		st2d_8to16table[i]= xlib_rgb16(palette[i*3], palette[i*3+1],palette[i*3+2]);
		st2d_8to24table[i]= xlib_rgb24(palette[i*3], palette[i*3+1],palette[i*3+2]);
	}

	if (x_visinfo->class == PseudoColor && x_visinfo->depth == 8)
	{
		if (palette != current_palette)
			memcpy(current_palette, palette, 768);
		for (i=0 ; i<256 ; i++)
		{
			colors[i].pixel = i;
			colors[i].flags = DoRed|DoGreen|DoBlue;
			colors[i].red = vid_gamma[palette[i*3]] * 257;
			colors[i].green = vid_gamma[palette[i*3+1]] * 257;
			colors[i].blue = vid_gamma[palette[i*3+2]] * 257;
		}
		XStoreColors(x_disp, x_cmap, colors, 256);
	}

}

// Called at shutdown

void	VID_Shutdown (void)
{
	Con_Printf("VID_Shutdown\n");
	//XAutoRepeatOn(x_disp);
	if (mouse_grabbed) {
		/* ungrab the pointer */
		XUngrabPointer(x_disp, CurrentTime);
		XUndefineCursor(x_disp, x_win);
	}
	XCloseDisplay(x_disp);
}

int XLateKey(XKeyEvent *ev)
{

	int key;
	char buf[64];
	KeySym keysym;

	XLookupString(ev, buf, sizeof buf, &keysym, 0);

	switch(keysym)
	{
		case XK_Page_Up:	 key = K_PGUP; break;
		case XK_Page_Down:	 key = K_PGDN; break;
		case XK_Home:	 key = K_HOME; break;
		case XK_End:	 key = K_END; break;
		case XK_Left:	 key = K_LEFTARROW; break;
		case XK_Right:	key = K_RIGHTARROW;		break;
		case XK_Down:	 key = K_DOWNARROW; break;
		case XK_Up:		 key = K_UPARROW;	 break;
		case XK_Escape: key = K_ESCAPE;		break;
		case XK_Return: key = K_ENTER;		 break;
		case XK_Tab:		key = K_TAB;			 break;
		case XK_F1:		 key = K_F1;				break;
		case XK_F2:		 key = K_F2;				break;
		case XK_F3:		 key = K_F3;				break;
		case XK_F4:		 key = K_F4;				break;
		case XK_F5:		 key = K_F5;				break;
		case XK_F6:		 key = K_F6;				break;
		case XK_F7:		 key = K_F7;				break;
		case XK_F8:		 key = K_F8;				break;
		case XK_F9:		 key = K_F9;				break;
		case XK_F10:		key = K_F10;			 break;
		case XK_F11:		key = K_F11;			 break;
		case XK_F12:		key = K_F12;			 break;
		case XK_BackSpace:
		case XK_Delete: key = K_BACKSPACE; break;
		case XK_Pause:	key = K_PAUSE;		 break;
		case XK_Shift_L:
		case XK_Shift_R:		key = K_SHIFT;		break;
		case XK_Control_L: 
		case XK_Control_R:	key = K_CTRL;		 break;
		case XK_Alt_L:	
		case XK_Meta_L: 
		case XK_Alt_R:	
		case XK_Meta_R: key = K_ALT;			break;
// various other keys on the keyboard
		case XK_F27: key = K_HOME; break;
		case XK_F29: key = K_PGUP; break;
		case XK_F33: key = K_END; break;
		case XK_F35: key = K_PGDN; break;
		case XK_KP_Insert: key = K_INS; break;

		default:
			key = *buf;
			break;
	} 

	return key;

}

struct
{
	int key;
	int down;
} keyq[64];
int keyq_head=0;
int keyq_tail=0;

int config_notify=0;
int config_notify_width;
int config_notify_height;

void GetEvent(void)
{
	XEvent x_event;

	XNextEvent(x_disp, &x_event);
	switch(x_event.type)
	{
		case KeyPress:
			Key_Event(XLateKey(&x_event.xkey), true);
			break;
		case KeyRelease:
			Key_Event(XLateKey(&x_event.xkey), false);
			break;
		case ButtonPress:
			//printf( "button %d down\n", x_event.xbutton.button );
			Key_Event( K_MOUSE1 + x_event.xbutton.button - 1, true );
			break;
		case ButtonRelease:
			//printf( "button %d up\n", x_event.xbutton.button );
			Key_Event( K_MOUSE1 + x_event.xbutton.button - 1, false );
			break;
		case MotionNotify:
			if (mouse_avail && mouse_grabbed) {
				mouse_x = (float) ((int)x_event.xmotion.x - (int)(vid.width/2));
				mouse_y = (float) ((int)x_event.xmotion.y - (int)(vid.height/2));
	//printf("m: x=%d,y=%d, mx=%3.2f,my=%3.2f\n", 
	//	x_event.xmotion.x, x_event.xmotion.y, mouse_x, mouse_y);

				/* move the mouse to the window center again */
				XSelectInput(x_disp,x_win, STD_EVENT_MASK & ~PointerMotionMask);
				XWarpPointer(x_disp,None,x_win,0,0,0,0, (vid.width/2),(vid.height/2));
				XSelectInput(x_disp,x_win, STD_EVENT_MASK);
			} else {
				mouse_x = (float) (x_event.xmotion.x-p_mouse_x);
				mouse_y = (float) (x_event.xmotion.y-p_mouse_y);
				p_mouse_x=x_event.xmotion.x;
				p_mouse_y=x_event.xmotion.y;
			}
			break;

		case ConfigureNotify:
//			printf("config notify\n");
			config_notify_width = x_event.xconfigure.width;
			config_notify_height = x_event.xconfigure.height;
			config_notify = 1;
			sb_updates = 0;
			break;
		case Expose:	
			sb_updates = 0;
			break;
		case ClientMessage:
			if (x_event.xclient.data.l[0] == aWMDelete) Host_Quit_f();
			break;
		case EnterNotify:
			mouse_in_window = true;
			break;
		case LeaveNotify:
			mouse_in_window = false;
			break;

		default:
			if (doShm && x_event.type == x_shmeventtype)
				oktodraw = true;
	}
   
	if (mouse_avail) {
		if (key_dest == key_game && !mouse_grabbed && mouse_in_window) {
			mouse_grabbed = true;
			/* grab the pointer */
			XGrabPointer(x_disp,x_win,True,0,GrabModeAsync,
				GrabModeAsync,x_win,None,CurrentTime);
			// inviso cursor
			XDefineCursor(x_disp, x_win, CreateNullCursor(x_disp, x_win));
		} else if ((key_dest != key_game || !mouse_in_window) && mouse_grabbed) {
			mouse_grabbed = false;
			/* ungrab the pointer */
			XUngrabPointer(x_disp, CurrentTime);
			XUndefineCursor(x_disp, x_win);
		}
	}
}

// flushes the given rectangles from the view buffer to the screen

void	VID_Update (vrect_t *rects)
{
#if 0
	static int count;
	static long long s;
	long long gethrtime();

	if (count == 0)
		s = gethrtime();

	if (count++ == 50) {
		count = 1;
		printf("%lf frames/secs\n", 50.0/((double)(gethrtime()-s) / 1e9));
		s = gethrtime();
	}
#endif

// if the window changes dimension, skip this frame

	if (config_notify)
	{
		printf("config notify\n");
		config_notify = 0;
		vid.width = config_notify_width & ~3;
		vid.height = config_notify_height;

		printf("w = %d, h = %d\n", vid.width, vid.height);

		if (doShm)
			ResetSharedFrameBuffers();
		else
			ResetFrameBuffer();
		vid.rowbytes = x_framebuffer[0]->bytes_per_line;
		vid.buffer = x_framebuffer[current_framebuffer]->data;
		vid.conbuffer = vid.buffer;
		vid.conwidth = vid.width;
		vid.conheight = vid.height;
		vid.conrowbytes = vid.rowbytes;
		vid.recalc_refdef = 1;				// force a surface cache flush
		return;
	}

	if (doShm)
	{
//		long long s, gethrtime();
//		s = gethrtime();

		while (rects)
		{
printf("update: %d,%d (%d,%d)\n", rects->x, rects->y, rects->width, rects->height);
			if (x_visinfo->depth == 16)
				st2_fixup( x_framebuffer[current_framebuffer], 
					rects->x, rects->y, rects->width,
					rects->height);
			else if (x_visinfo->depth == 24)
				st3_fixup( x_framebuffer[current_framebuffer], 
					rects->x, rects->y, rects->width,
					rects->height);
			if (!XShmPutImage(x_disp, x_win, x_gc,
				x_framebuffer[current_framebuffer], rects->x, rects->y,
				rects->x, rects->y, rects->width, rects->height, True))
					Sys_Error("VID_Update: XShmPutImage failed\n");
			oktodraw = false;
			while (!oktodraw) GetEvent();
			rects = rects->pnext;
		}
//		printf("%lf\n", (double)(gethrtime()-s)/1.0e9);
		current_framebuffer = !current_framebuffer;
		vid.buffer = x_framebuffer[current_framebuffer]->data;
		vid.conbuffer = vid.buffer;
		XSync(x_disp, False);
		
	}
	else
	{
		while (rects)
		{
			if (x_visinfo->depth == 16)
				st2_fixup( x_framebuffer[current_framebuffer], 
					rects->x, rects->y, rects->width,
					rects->height);
			else if (x_visinfo->depth == 24)
				st3_fixup( x_framebuffer[current_framebuffer], 
					rects->x, rects->y, rects->width,
					rects->height);
			XPutImage(x_disp, x_win, x_gc, x_framebuffer[0], rects->x,
				rects->y, rects->x, rects->y, rects->width, rects->height);
			rects = rects->pnext;
		}
		XSync(x_disp, False);
	}
}

static int dither;

void VID_DitherOn(void)
{
    if (dither == 0)
    {
		vid.recalc_refdef = 1;
        dither = 1;
    }
}

void VID_DitherOff(void)
{
    if (dither)
    {
		vid.recalc_refdef = 1;
        dither = 0;
    }
}

void VID_SetDefaultMode( void )
{
}

int I_OpenWindow(void)
{
	return 0;
}

void I_EraseWindow(int window)
{
}

void I_DrawCircle(int window, int x, int y, int r)
{
}

void I_DisplayWindow(int window)
{
}

void Sys_SendKeyEvents(void)
{
// get events from x server
	if (x_disp)
	{
		while (XPending(x_disp)) GetEvent();
		while (keyq_head != keyq_tail)
		{
			Key_Event(keyq[keyq_tail].key, keyq[keyq_tail].down);
			keyq_tail = (keyq_tail + 1) & 63;
		}
	}
}

#if 0
char *Sys_ConsoleInput (void)
{

	static char	text[256];
	int		len;
	fd_set  readfds;
	int		ready;
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO(&readfds);
	FD_SET(0, &readfds);
	ready = select(1, &readfds, 0, 0, &timeout);

	if (ready>0)
	{
		len = read (0, text, sizeof(text));
		if (len >= 1)
		{
			text[len-1] = 0;	// rip off the /n and terminate
			return text;
		}
	}

	return 0;
	
}
#endif

void IN_Init (void)
{
	Cvar_RegisterVariable (&m_filter);
	if ( COM_CheckParm ("-nomouse") )
		return;
	mouse_x = mouse_y = 0.0;
	mouse_avail = 1;
}

void IN_Shutdown (void)
{
	mouse_avail = 0;
}

void IN_Commands (void)
{
	int i;
   
	if (!mouse_avail) return;
   
	for (i=0 ; i<mouse_buttons ; i++) {
		if ( (mouse_buttonstate & (1<<i)) && !(mouse_oldbuttonstate & (1<<i)) )
			Key_Event (K_MOUSE1 + i, true);

		if ( !(mouse_buttonstate & (1<<i)) && (mouse_oldbuttonstate & (1<<i)) )
			Key_Event (K_MOUSE1 + i, false);
	}
	mouse_oldbuttonstate = mouse_buttonstate;
}

void IN_Move (usercmd_t *cmd)
{
	if (!mouse_avail)
		return;
   
	if (m_filter.value) {
		mouse_x = (mouse_x + old_mouse_x) * 0.5;
		mouse_y = (mouse_y + old_mouse_y) * 0.5;
	}

	old_mouse_x = mouse_x;
	old_mouse_y = mouse_y;
   
	mouse_x *= sensitivity.value;
	mouse_y *= sensitivity.value;
   
	if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
		cmd->sidemove += m_side.value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;
	if (in_mlook.state & 1)
		V_StopPitchDrift ();
   
	if ( (in_mlook.state & 1) && !(in_strafe.state & 1)) {
		cl.viewangles[PITCH] += m_pitch.value * mouse_y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	} else {
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}
	mouse_x = mouse_y = 0.0;
}
