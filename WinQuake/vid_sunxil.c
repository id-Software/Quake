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
// vid_sunxil.c -- uses X to setup windows and XIL to copy images (scaled as needed) 
// 		   to screen

#define _BSD
#define BYTE_DEFINED 1

#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <thread.h>
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
#include <xil/xil.h>

#include "quakedef.h"
#include "d_local.h"

#define MIN_WIDTH 320
#define MIN_HEIGHT 200

cvar_t		_windowed_mouse = {"_windowed_mouse","0", true};
cvar_t		m_filter = {"m_filter","0", true};
float old_windowed_mouse;

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

qboolean        mouse_avail;
int             mouse_buttons=3;
int             mouse_oldbuttonstate;
int             mouse_buttonstate;
float   mouse_x, mouse_y;
float   old_mouse_x, old_mouse_y;
int p_mouse_x;
int p_mouse_y;

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
( \
	StructureNotifyMask | \
	KeyPressMask | \
	KeyReleaseMask | \
	ButtonPressMask | \
	ButtonReleaseMask | \
	ExposureMask | \
	PointerMotionMask | \
	FocusChangeMask \
)

int		VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes, VGA_planar;
byte	*VGA_pagebase;

qboolean			x_fullscreen = true;
Display				*x_disp = NULL;
int				x_screen, x_screen_width, x_screen_height;
int				x_center_width, x_center_height;
int				x_std_event_mask = STD_EVENT_MASK;
Window				x_win, x_root_win;
qboolean			x_focus = true;
int				global_dx, global_dy;


static Colormap			x_cmap;
static GC			x_gc;
static Visual			*x_vis;
static XVisualInfo		*x_visinfo;
static Atom			aHints = NULL;
static Atom			aWMDelete = NULL;

static qboolean			oktodraw = false;
static qboolean			X11_active = false;


static int verbose=1;

static byte current_palette[768];

cvar_t pixel_multiply = {"pixel_multiply", "2", true};
int current_pixel_multiply = 2;

#define PM(a) (int)((current_pixel_multiply)?((a)*current_pixel_multiply):(a))
#define MP(a) (int)((current_pixel_multiply)?((a)/current_pixel_multiply):(a))

static int 				render_pipeline[2];
static XilSystemState 			state;
static XilImage				display_image  = NULL;
static XilImage				quake_image  = NULL;
static int				use_mt = 0;
static int				count_frames = 0;

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
// direct drawing of the "accessing disk" icon isnt supported under Nextstep
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

	if (Cmd_Argc () == 2) {
		g = Q_atof (Cmd_Argv(1));

		for (i=0 ; i<255 ; i++)	{
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

qboolean CheckPixelMultiply (void)
{
	int m;
	int w, h;
	XWindowAttributes wattr;   
	XWindowChanges chg;
	unsigned int value_mask;
	int old_pixel;

	if ((m = (int)pixel_multiply.value) != current_pixel_multiply) {
		if (m < 1)
			m = 1;
		if (m > 4)
			m = 4;

		old_pixel = current_pixel_multiply;
		current_pixel_multiply = m;
		Cvar_SetValue("pixel_multiply", m);
		
		if(XGetWindowAttributes(x_disp, x_win, & wattr) == 0)
			return true; // ???

		memset(&chg, 0, sizeof(chg));
		chg.width = wattr.width/old_pixel * current_pixel_multiply;
		chg.height = wattr.height/old_pixel * current_pixel_multiply;

		if (chg.width < MIN_WIDTH*current_pixel_multiply)
			chg.width = MIN_WIDTH*current_pixel_multiply;
		if (chg.height < MIN_HEIGHT*current_pixel_multiply)
			chg.height = MIN_HEIGHT*current_pixel_multiply;

		XConfigureWindow(x_disp, x_win, CWWidth | CWHeight, &chg);

		vid.width = MP(wattr.width) & ~3;
		vid.height = MP(wattr.height);

		if (vid.width < 320) 
			vid.width = 320;
		if (vid.height < 200)
			vid.height = 200;
		VID_ResetFramebuffer();

		return true;
	}
	return false;
}

// ========================================================================
// Tragic death handler
// ========================================================================

void TragicDeath(int signal_num)
{
	//XAutoRepeatOn(x_disp);
	XCloseDisplay(x_disp);
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
	if (aHints == None) {
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
	int w, h;

	int desired_width=320, desired_height=200;
	
	Cmd_AddCommand ("gamma", VID_Gamma_f);

	Cvar_RegisterVariable (&pixel_multiply);

	if (pipe(render_pipeline) < 0) 
		Sys_Error("VID_Init: pipe");
	
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

	verbose = COM_CheckParm("-verbose");
	count_frames = COM_CheckParm("-count_frames");

//
// open the display
//
	x_disp = XOpenDisplay(0);

	if (!x_disp) {
		if (getenv("DISPLAY"))
			Sys_Error("VID: Could not open display [%s]\n",
				getenv("DISPLAY"));
		else
			Sys_Error("VID: Could not open local display\n");
	}

	x_screen = DefaultScreen( x_disp );
	x_screen_width = WidthOfScreen( ScreenOfDisplay( x_disp, x_screen ) );
	x_screen_height = HeightOfScreen( ScreenOfDisplay( x_disp, x_screen ) );

	x_center_width  = x_screen_width/2;

	x_center_height = x_screen_height/2;

	Con_Printf( "Using screen %d: %dx%d\n", x_screen, x_screen_width, x_screen_height );

	x_root_win = DefaultRootWindow( x_disp);

	//XAutoRepeatOff(x_disp);

// for debugging only
	if (verbose)
		XSynchronize(x_disp, True);

//
// check for command-line window size
//
	if ((pnum=COM_CheckParm("-winsize"))) {
		if (pnum >= com_argc-2)
			Sys_Error("VID: -winsize <width> <height>\n");
		desired_width = Q_atoi(com_argv[pnum+1]);
		desired_height = Q_atoi(com_argv[pnum+2]);
		if (desired_width < 1 || desired_height < 1)
			Sys_Error("VID: Bad window width/height\n");		
	}

	template_mask = VisualScreenMask; // make sure we get the right one	
	template.screen = x_screen;
//
// specify a visual id
//
	if ((pnum=COM_CheckParm("-visualid"))) {
		if (pnum >= com_argc-1)
			Sys_Error("VID: -visualid <id#>\n");
		template.visualid = Q_atoi(com_argv[pnum+1]);
		template_mask |= VisualIDMask;
	} else	{
		// If not specified, find an 8 bit visual since others don't work
//		template.depth = 8;
//		template_mask |= VisualDepthMask;
		int screen;
		screen = XDefaultScreen(x_disp);
		template.visualid =
			XVisualIDFromVisual(XDefaultVisual(x_disp, screen));
		template_mask = VisualIDMask;
	}
//
// pick a visual- warn if more than one was available
//
	x_visinfo = XGetVisualInfo(x_disp, template_mask, &template, &num_visuals);
	if (num_visuals > 1) {
		printf("Found more than one visual id at depth %d:\n", template.depth);
		for (i=0 ; i<num_visuals ; i++)
			printf("	-visualid %d\n", (int)(x_visinfo[i].visualid));
	}
	else if (num_visuals == 0) {
		if (template_mask == VisualIDMask)
			Sys_Error("VID: Bad visual id %d\n", template.visualid);
		else
			Sys_Error("VID: No visuals at depth %d\n", template.depth);
	}

	if (verbose) {
		printf("Using visualid %d:\n", (int)(x_visinfo->visualid));
		printf("	screen %d\n", x_visinfo->screen);
		printf("	red_mask 0x%x\n", (int)(x_visinfo->red_mask));
		printf("	green_mask 0x%x\n", (int)(x_visinfo->green_mask));
		printf("	blue_mask 0x%x\n", (int)(x_visinfo->blue_mask));
		printf("	colormap_size %d\n", x_visinfo->colormap_size);
		printf("	bits_per_rgb %d\n", x_visinfo->bits_per_rgb);
	}

	x_vis = x_visinfo->visual;
//
// See if we're going to do pixel multiply
//
	if (pixel_multiply.value < 1 || pixel_multiply.value > 4)
		Cvar_SetValue("pixel_multiply", 2);
	current_pixel_multiply = pixel_multiply.value;

	w = 320*current_pixel_multiply; // minimum width
	h = 200*current_pixel_multiply; // minimum height
	if (desired_width < w)
		desired_width = w;
	if (desired_height < h)
		desired_height = h;

	vid.width = MP(desired_width);
	vid.height = MP(desired_height);

	//
	// patch things up so game doesn't fail if window is too small
	//

	if (vid.width < 320) 
		vid.width = 320;
	if (vid.height < 200)
		vid.height = 200;

//
// see if we're going to use threads
//
	if(((sysconf(_SC_NPROCESSORS_ONLN) > 1) || COM_CheckParm("-mt")) &&  
		(COM_CheckParm("-no_mt") == 0)) {
		use_mt = 1;
		printf("VID: Using multiple threads!\n");
	}

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
			desired_width, desired_height,
			0, // borderwidth
			x_visinfo->depth,
			InputOutput,
			x_vis,
			attribmask,
			&attribs );

		if (x_visinfo->class != TrueColor)
			XFreeColormap(x_disp, tmpcmap);

	}

	if (x_visinfo->depth == 8) {

	// create and upload the palette
		if (x_visinfo->class == PseudoColor) {
			x_cmap = XCreateColormap(x_disp, x_win, x_vis, AllocAll);
			VID_SetPalette(palette);
			XSetWindowColormap(x_disp, x_win, x_cmap);
		}

	}

	VID_SetWindowTitle( x_win, "Quake" );

// inviso cursor
	XDefineCursor(x_disp, x_win, CreateNullCursor(x_disp, x_win));

// create the GC
	{
		XGCValues xgcvalues;
		int valuemask = GCGraphicsExposures;
		xgcvalues.graphics_exposures = False;
		x_gc = XCreateGC(x_disp, x_win, valuemask, &xgcvalues );
	}

// map the window
	XMapWindow(x_disp, x_win);
	XSync(x_disp, True) ;        /* wait for map */
//
// wait for first exposure event
//
	{
		XEvent event;
		do{
			XNextEvent(x_disp, &event);
			if (event.type == Expose && !event.xexpose.count)
				oktodraw = true;
		} while (!oktodraw);
	}
//
// initialize XIL
//
	
	state = xil_open();
	
	if(state == NULL) {
		//
		//  XIL's default error handler will print an error msg on stderr
		//
		Sys_Error("xil_open failed\n");
	}
	
	X11_active = true;

	VID_ResetFramebuffer();

	D_InitCaches (surfcache, sizeof(surfcache));
	
	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;
}

VID_ResetFramebuffer()
{
	XilMemoryStorage storage;

	if (use_mt) {
		VID_ResetFramebuffer_MT();
		return;
	}

//printf("VID_ResetFramebuffer: vid.width %d, vid.height %d\n", vid.width, vid.height);

	xil_destroy(display_image);

	xil_destroy(quake_image);

	display_image = xil_create_from_window(state, x_disp, x_win);
	quake_image = xil_create(state, vid.width, vid.height, 1, XIL_BYTE);

	xil_export(quake_image);
	
	if (xil_get_memory_storage(quake_image, &storage) == FALSE)
		Sys_Error("xil_get_memory_storage");
	
	xil_import(quake_image, TRUE);
	xil_export(quake_image);

	if (xil_get_memory_storage(quake_image, &storage) == FALSE)
		Sys_Error("xil_get_memory_storage");

	vid.rowbytes = storage.byte.scanline_stride;
	vid.buffer =   storage.byte.data;
	vid.conbuffer = vid.buffer;
	vid.conrowbytes = vid.rowbytes;
	vid.conwidth = vid.width;
	vid.conheight = vid.height;
	
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.recalc_refdef = 1;				// force a surface cache flush

	free(d_pzbuffer);

	d_pzbuffer = malloc(PM(vid.width)*PM(vid.height)*sizeof(*d_pzbuffer));
        //Hunk_HighAllocName(PM(vid.width)*PM(vid.height)*sizeof(*d_pzbuffer),"zbuff");
}

VID_ResetFramebuffer_MT()
{
	XilMemoryStorage storage;
	XilImage drain_renderpipeline();
	XilImage old_display_image;

	void * update_thread();

	printf("VID_ResetFramebuffer: vid.width %d, vid.height %d\n", vid.width, vid.height);

	old_display_image = display_image;

	display_image = xil_create_from_window(state, x_disp, x_win);
	
	if (quake_image == NULL) 
		if (thr_create(NULL, NULL, update_thread, NULL, THR_NEW_LWP, NULL) != 0)
			Sys_Error("VID: thr_create");	
	
	quake_image = drain_renderpipeline(quake_image);

	xil_destroy(old_display_image);

	free(d_pzbuffer);

	d_pzbuffer = malloc(PM(vid.width)*PM(vid.height)*sizeof(*d_pzbuffer));
}

void VID_ShiftPalette(unsigned char *p)
{
	VID_SetPalette(p);
}

void VID_SetPalette(unsigned char *palette)
{

	int i;
	XColor colors[256];

	if (x_visinfo->class == PseudoColor && x_visinfo->depth == 8) {
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
	X11_active = false;
	Con_Printf("VID_Shutdown\n");
	//XAutoRepeatOn(x_disp);
	xil_destroy(display_image);
	xil_destroy(quake_image);
	display_image = NULL;
	quake_image = NULL;
	XCloseDisplay(x_disp);
}

int XLateKey(XKeyEvent *ev)
{

	int key;
	char buf[64];
	KeySym keysym;

	XLookupString(ev, buf, sizeof buf, &keysym, 0);

	switch(keysym) {
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
		case XK_Help:
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
		case XK_Insert:
		case XK_KP_Insert: key = K_INS; break;
		case XK_F24: key = '-'; break;
		case XK_KP_Add: key = '+'; break;
		case XK_KP_Subtract: key = '-'; break;
		case XK_F25: key = '/'; break;
		case XK_F26: key = '*'; break;

		default:
			key = (unsigned char)*buf;
			break;
		} 

	return key;

}

struct {
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
	int b;

	XNextEvent(x_disp, &x_event);
	switch(x_event.type) {
		case KeyPress:
			Key_Event(XLateKey(&x_event.xkey), true);
			break;
		case KeyRelease:
			Key_Event(XLateKey(&x_event.xkey), false);
			break;

		case MotionNotify:

			if (_windowed_mouse.value) {
				mouse_x = (float) ((int)x_event.xmotion.x - (int)(vid.width/2));
				mouse_y = (float) ((int)x_event.xmotion.y - (int)(vid.height/2));
	//printf("m: x=%d,y=%d, mx=%3.2f,my=%3.2f\n", 
	//	x_event.xmotion.x, x_event.xmotion.y, mouse_x, mouse_y);

				/* move the mouse to the window center again */
				XSelectInput( x_disp, x_win, x_std_event_mask & ~PointerMotionMask );
				XWarpPointer(x_disp,None,x_win,0,0,0,0, 
					(vid.width/2),(vid.height/2));
				XSelectInput( x_disp, x_win, x_std_event_mask );
			} else {
				mouse_x = (float) (x_event.xmotion.x-p_mouse_x);
				mouse_y = (float) (x_event.xmotion.y-p_mouse_y);
				p_mouse_x=x_event.xmotion.x;
				p_mouse_y=x_event.xmotion.y;
			}
			break;

		case ButtonPress:
			b=-1;
			if (x_event.xbutton.button == 1)
				b = 0;
			else if (x_event.xbutton.button == 2)
				b = 2;
			else if (x_event.xbutton.button == 3)
				b = 1;
			if (b>=0)
				mouse_buttonstate |= 1<<b;
			break;

		case ButtonRelease:
			b=-1;
			if (x_event.xbutton.button == 1)
				b = 0;
			else if (x_event.xbutton.button == 2)
				b = 2;
			else if (x_event.xbutton.button == 3)
				b = 1;
			if (b>=0)
				mouse_buttonstate &= ~(1<<b);
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
#if 0
		case FocusIn:
			printf("FocusIn...\n");
			x_focus = true;
			break;
		case FocusOut:
			printf("FocusOut...\n");
			x_focus = false;
			break;
#endif
	}

	if (old_windowed_mouse != _windowed_mouse.value) {
		old_windowed_mouse = _windowed_mouse.value;

		if (!_windowed_mouse.value) {
			/* ungrab the pointer */
			XUngrabPointer(x_disp,CurrentTime);
		} else {
			/* grab the pointer */
			XGrabPointer(x_disp,x_win,True,0,GrabModeAsync,
				GrabModeAsync,x_win,None,CurrentTime);
		}
	}
}

// flushes the given rectangles from the view buffer to the screen

void
VID_Update (vrect_t *rects)
{
	void VID_Update_MT(vrect_t *);


	if (count_frames) {
		static int count;
		static long long s;
		long long gethrtime();

		if (count == 0)
			s = gethrtime();

		if (count++ == 200) {
			long long n = gethrtime();
			count = 1;
			printf("%lf frames/secs\n", 200.0/((double)(n-s) / 1e9));
			s = n;
		}
	}

	if (use_mt) {
		VID_Update_MT(rects);
		return;
	}

	// if the window changes dimension, skip this frame

	if (config_notify) {
		int w, h;
		XWindowChanges chg;
		unsigned int value_mask;

		w = 320*current_pixel_multiply; // minimum width
		h = 200*current_pixel_multiply; // minimum height
			
		if (config_notify_width < w || config_notify_height < h) {
			// We must resize the window
			memset(&chg, 0, sizeof(chg));
			value_mask = 0;
			if (config_notify_width < w) {
				config_notify_width = chg.width = w;
				value_mask |= CWWidth;
			}
			if (config_notify_height < h) {
				config_notify_height = chg.height = h;
				value_mask |= CWHeight;
			}
			if (value_mask)
				XConfigureWindow(x_disp, x_win, value_mask, &chg);
		}

		config_notify = 0;

		vid.width = MP(config_notify_width) & ~3;
		vid.height = MP(config_notify_height);

		if (vid.width < 320) 
			vid.width = 320;
		if (vid.height < 200)
			vid.height = 200;
		
		VID_ResetFramebuffer();

		return;
	}
	// if pixel multiply changed, skip this frame
	if (CheckPixelMultiply())
		return;

	while (rects) { // I've never seen more than one rect?
		XilMemoryStorage storage;

		xil_import(quake_image, TRUE); // let xil control the image

		if (current_pixel_multiply < 2)
			xil_copy(quake_image, display_image);
		else
			xil_scale(quake_image, display_image, "nearest",
				  (float)current_pixel_multiply, (float)current_pixel_multiply);

		xil_export(quake_image);  // back to quake

		if (xil_get_memory_storage(quake_image, &storage) == FALSE)
			Sys_Error("xil_get_memory_storage");

		vid.buffer =   storage.byte.data;
		vid.conbuffer = vid.buffer;

		rects = rects->pnext;
	}
}

void
VID_Update_MT (vrect_t *rects)
{
	XilImage sched_update();

	// if the window changes dimension, skip this frame

	if (config_notify) {
		int w, h;
		XWindowChanges chg;
		unsigned int value_mask;

		w = 320*current_pixel_multiply; // minimum width
		h = 200*current_pixel_multiply; // minimum height
			
		if (config_notify_width < w || config_notify_height < h) {
			// We must resize the window
			memset(&chg, 0, sizeof(chg));
			value_mask = 0;
			if (config_notify_width < w) {
				config_notify_width = chg.width = w;
				value_mask |= CWWidth;
			}
			if (config_notify_height < h) {
				config_notify_height = chg.height = h;
				value_mask |= CWHeight;
			}
			if (value_mask)
				XConfigureWindow(x_disp, x_win, value_mask, &chg);
		}

		config_notify = 0;

		vid.width = MP(config_notify_width) & ~3;
		vid.height = MP(config_notify_height);

		if (vid.width < 320) 
			vid.width = 320;
		if (vid.height < 200)
			vid.height = 200;
		
		VID_ResetFramebuffer_MT();

		return;
	}
	// if pixel multiply changed, skip this frame
	if (CheckPixelMultiply())
		return;

	quake_image = sched_update(quake_image);
}

XilImage
drain_renderpipeline(XilImage old)
{
	XilImage new;

	XilMemoryStorage storage;
	
	if (old) 
		if (read(render_pipeline[1], &new, sizeof(new)) != sizeof (new)) {
			Sys_Error("drain_renderpipeline: read");
			xil_destroy(new);
		}

	xil_destroy(old);


	new = xil_create(state, vid.width, vid.height, 1, XIL_BYTE);
	
	if (write(render_pipeline[0], &new, sizeof (new)) != sizeof(new)) 
		Sys_Error("drain_renderpipeline: write");

	new = xil_create(state, vid.width, vid.height, 1, XIL_BYTE);

	xil_export(new);

	if (xil_get_memory_storage(new, &storage) == FALSE)
		Sys_Error("xil_get_memory_storage");

	vid.rowbytes = storage.byte.scanline_stride;
	vid.buffer =   storage.byte.data;
	vid.conbuffer = vid.buffer;
	vid.conrowbytes = vid.rowbytes;
	vid.conwidth = vid.width;
	vid.conheight = vid.height;
	
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.recalc_refdef = 1;				// force a surface cache flush

	return(new);

}

XilImage
sched_update(XilImage image)
{
	XilImage new;
	XilMemoryStorage storage;

	if (write(render_pipeline[1], &image, sizeof(image)) != sizeof (image)) 
		Sys_Error("sched_update:write");

	if (read(render_pipeline[1], &new, sizeof(new)) != sizeof (new))
		Sys_Error("sched_update:read");

	xil_export(new);

	if (xil_get_memory_storage(new, &storage) == FALSE)
		Sys_Error("xil_get_memory_storage");

	vid.buffer =   storage.byte.data;
	vid.conbuffer = vid.buffer;

	return (new);
}

void *update_thread()
{
	XilImage image;

	if (!X11_active)
		return;

	while (read(render_pipeline[0], &image, sizeof (image)) == sizeof(image)) {

		xil_import(image, TRUE); // let xil control the image

		if (!display_image)
			return;

		if (current_pixel_multiply < 2)
			xil_copy(image, display_image);
		else
			xil_scale(image, display_image, "nearest",
				  (float)current_pixel_multiply, (float)current_pixel_multiply);

		if (write(render_pipeline[0], &image, sizeof (image)) != sizeof(image)) 
			Sys_Error("update_thread: write");
	}
}


static int dither;

void VID_DitherOn(void)
{
	if (dither == 0) {
		vid.recalc_refdef = 1;
		dither = 1;
	}
}

void VID_DitherOff(void)
{
	if (dither) {
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
		   if (x_disp) {
				   while (XPending(x_disp)) GetEvent();
				   while (keyq_head != keyq_tail) {
						   Key_Event(keyq[keyq_tail].key, keyq[keyq_tail].down);
						   keyq_tail = (keyq_tail + 1) & 63;
					   }
			   }
}

void IN_Init (void)
{
	Cvar_RegisterVariable (&_windowed_mouse);
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

//void VID_UnlockBuffer(void) { }
//void VID_LockBuffer(void) { }

