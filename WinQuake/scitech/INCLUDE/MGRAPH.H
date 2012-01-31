/****************************************************************************
*
*						MegaGraph Graphics Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   mgraph.h  $
* Version:		$Revision:   1.29  $
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for the MegaGraph Graphics Library. You can
*				defined one of the following to specify which MGL API you
*				wish to use.
*
*					MGL_LITE	- Compile for the MGL/Lite API
*					MGL_PRO		- Compile for the MGL/Pro API
*					MGL_FIX3D	- Compile for the MGL/3D API (fixed point)
*					MGL_FLT3D	- Compile for the MGL/3D API (floating point)
*
*				If you do not define any of these, MGL_FIX3D will be defined
*				automatically for compatibility with older versions of the
*				MGL.
*
* $Date:   11 Mar 1997 16:46:42  $ $Author:   KendallB  $
*
****************************************************************************/

#ifndef	__MGRAPH_H
#define	__MGRAPH_H

#include <stdio.h>

#ifndef	__DEBUG_H
#include "debug.h"
#endif

#if	!defined(MGL_LITE) && !defined(MGL_PRO) && !defined(MGL_FIX3D) \
	&& !defined(MGL_FLT3D)
#define	MGL_FIX3D
#endif

#if	defined(MGL_FIX3D) || defined(MGL_FLT3D)
#define	MGL_3D
#endif

/*---------------------- Macros and type definitions ----------------------*/

#pragma pack(1)				/* Pack structures to byte granularity		*/

/* Define the version number for the MGL release */

#define MGL_VERSION_STR     "3.1"

/* Define the calling conventions for all public MGL functions. If we
 * are compiling the MGL as a DLL, then all public functions are compiled
 * and exported with standard C calling conventions, otherwise we use
 * the default calling conventions provided by the compiler.
 *
 * Note that because Watcom C++ uses register based parameter passing
 * by default we also provide special DLL's for watcom that are compiled
 * with register parameter passing. This is necessary to work with all
 * the extra libraries provided as they are all compiled to call MGL
 * functions with arguments in registers whenever possible. You can still
 * use the standard DLL but if you do you will need to re-compile all
 * of the extra libraries with the MGL_DLL #define to change the calling
 * conventions for the MGL functions.
 */

#if		defined(MGL_DLL)
#define	MGLAPI	_EXPORT __cdecl
#else
#define	MGLAPI	_EXPORT _PUBAPI
#endif
#define	ASMAPI	_EXPORT __cdecl

/* Define a type for integers used in the library. For most environments
 * we simply define it as a normal integer (16 or 32 bits), however for
 * 16 bit Windows it is defined as a 32 bit integer as all the real code
 * lives in a 32 bit DLL that uses 32 bit integers.
 */

#ifndef	__M_INT_DEFINED
#if	defined(__WINDOWS16__)
typedef	long			m_int;
typedef unsigned long	m_uint;
#else
typedef int				m_int;
typedef	unsigned int	m_uint;
#endif
#define	__M_INT_DEFINED
#endif

/* Define the graphics subsystems available	*/

typedef enum {
	grDETECT		= -1,	/* Auto detect the graphics subsystem		*/
	grNONE			= 0,	/* No graphics hardware detected			*/
	grVGA,					/* Standard VGA								*/
	grVESA,					/* VESA VBE compliant SuperVGA				*/
	grSVGA,					/* Unaccelerated SuperVGA					*/
	grACCEL,				/* Accelerated SuperVGA						*/
	grDDRAW,				/* Unaccelerated DirectDraw					*/
	grDDRAWACCEL,			/* Accelerated DirectDraw 					*/
	grDDRAW3D,				/* 3D Accelerated DirectDraw 				*/
	grMAXDRIVER,			/* Maximum driver number					*/
	} MGL_driverType;

/* Graphics modes supported	- the only video modes supported by this
 * graphics library are those that support at least 16 colors per pixel.
 */

typedef enum {
	/* 16 color VGA video modes */

	grVGA_320x200x16,
	grVGA_640x200x16,
	grVGA_640x350x16,
	grVGA_640x400x16,
	grVGA_640x480x16,
	grSVGA_800x600x16,

	/* 256 color VGA ModeX video modes */

	grVGAX_320x200x256,
	grVGAX_320x240x256,
	grVGAX_320x400x256,
	grVGAX_320x480x256,

	/* 256 color VGA video modes */

	grVGA_320x200x256,

	/* 256 color VGA/SuperVGA video modes */

	grSVGA_320x200x256,
	grSVGA_320x240x256,
	grSVGA_320x400x256,
	grSVGA_320x480x256,
	grSVGA_400x300x256,
	grSVGA_512x384x256,
	grSVGA_640x350x256,
	grSVGA_640x400x256,
	grSVGA_640x480x256,
	grSVGA_800x600x256,
	grSVGA_1024x768x256,
	grSVGA_1152x864x256,
	grSVGA_1280x960x256,
	grSVGA_1280x1024x256,
	grSVGA_1600x1200x256,

	/* 32,768 color Super VGA video modes */

    grSVGA_320x200x32k,
	grSVGA_320x240x32k,
	grSVGA_320x400x32k,
	grSVGA_320x480x32k,
	grSVGA_400x300x32k,
	grSVGA_512x384x32k,
	grSVGA_640x350x32k,
    grSVGA_640x400x32k,
    grSVGA_640x480x32k,
    grSVGA_800x600x32k,
    grSVGA_1024x768x32k,
	grSVGA_1152x864x32k,
	grSVGA_1280x960x32k,
	grSVGA_1280x1024x32k,
    grSVGA_1600x1200x32k,

	/* 65,536 color Super VGA video modes */

    grSVGA_320x200x64k,
	grSVGA_320x240x64k,
	grSVGA_320x400x64k,
	grSVGA_320x480x64k,
	grSVGA_400x300x64k,
	grSVGA_512x384x64k,
	grSVGA_640x350x64k,
	grSVGA_640x400x64k,
	grSVGA_640x480x64k,
	grSVGA_800x600x64k,
	grSVGA_1024x768x64k,
	grSVGA_1152x864x64k,
	grSVGA_1280x960x64k,
	grSVGA_1280x1024x64k,
	grSVGA_1600x1200x64k,

	/* 16 million color, 24 bits per pixel Super VGA video modes */

	grSVGA_320x200x16m,
	grSVGA_320x240x16m,
	grSVGA_320x400x16m,
	grSVGA_320x480x16m,
	grSVGA_400x300x16m,
	grSVGA_512x384x16m,
	grSVGA_640x350x16m,
	grSVGA_640x400x16m,
	grSVGA_640x480x16m,
	grSVGA_800x600x16m,
	grSVGA_1024x768x16m,
	grSVGA_1152x864x16m,
	grSVGA_1280x960x16m,
	grSVGA_1280x1024x16m,
	grSVGA_1600x1200x16m,

	/* 16 million color, 32 bits per pixel Super VGA video modes */

	grSVGA_320x200x4G,
	grSVGA_320x240x4G,
	grSVGA_320x400x4G,
	grSVGA_320x480x4G,
	grSVGA_400x300x4G,
	grSVGA_512x384x4G,
	grSVGA_640x350x4G,
	grSVGA_640x400x4G,
	grSVGA_640x480x4G,
	grSVGA_800x600x4G,
	grSVGA_1024x768x4G,
	grSVGA_1152x864x4G,
	grSVGA_1280x960x4G,
	grSVGA_1280x1024x4G,
	grSVGA_1600x1200x4G,

	/* Render into Windowing System DC (Windows, OS/2 PM, X11) */

	grWINDOWED,

	grMAXMODE,					/* Maximum mode number					*/
	} MGL_modeType;

/* MGL_result() error codes	*/

typedef enum {
	grOK			= 0,	/* No error									*/
	grNoInit		= -1,	/* Graphics driver has not been installed	*/
	grNotDetected	= -2,	/* Graphics hardware was not detected		*/
	grDriverNotFound= -3,	/* Graphics driver file not found			*/
	grBadDriver		= -4,	/* File loaded was not a graphics driver	*/
	grLoadMem		= -5,	/* Not enough memory to load graphics driver*/
	grInvalidMode	= -6,	/* Invalid graphics mode for selected driver*/
	grError			= -8,	/* General graphics error					*/
	grInvalidName	= -9,	/* Invalid driver name						*/
	grNoMem			= -10,	/* Not enough memory to perform operation	*/
	grNoModeSupport	= -11,	/* Select video mode not supported by hard.	*/
	grInvalidFont	= -12,	/* Invalid font data						*/
	grBadFontFile	= -13,	/* File loaded was not a font file			*/
	grFontNotFound	= -14,	/* Font file was not found					*/
	grOldDriver     = -15,	/* Driver file is an old version			*/
	grInvalidDevice	= -16,	/* Invalid device for selected operation	*/
	grInvalidDC		= -17,	/* Invalid device context					*/
	grInvalidCursor	= -18,	/* Invalid cursor file						*/
	grCursorNotFound= -19,	/* Cursor file was not found				*/
	grInvalidIcon	= -20,	/* Invalid icon file						*/
	grIconNotFound	= -21,	/* Icon file was not found					*/
	grInvalidBitmap = -22,	/* Invalid bitmap file						*/
	grBitmapNotFound= -23,	/* Bitmap file was not found				*/
	grZbufferTooBig	= -24,	/* Zbuffer allocation is too large			*/
	grNewFontFile	= -25,	/* Only Windows 2.x font files supported	*/
	grNoDoubleBuff  = -26,	/* Double buffering is not available		*/
	grNoHardwareBlt	= -28,	/* No hardware bitBlt for OffscreenDC		*/
	grNoOffscreenMem= -29,	/* No available offscreen memory			*/
	grInvalidPF		= -30,	/* Invalid pixel format for memory DC		*/

	grLastError		= -31,	/* Last error number in list				*/
	} MGL_errorType;

#define	MGL_CLIPON		true
#define	MGL_CLIPOFF		false

/* Color mapped modes */

typedef enum {
	MGL_CMAP_MODE,					/* Normal Color mapped mode 	*/
	MGL_DITHER_RGB_MODE,			/* 24 bit RGB halftone dithered	*/
	} MGL_colorModes;

/* Standard colors - this is the standard set of colors for the IBM PC. The
 * default palette will have been programmed to contain these values when a
 * graphics modes is started. If the palette has been changed, they will
 * not correspond to the actual colors on the screen. Under a Windowing
 * manage environment these colors will also not be setup by default.
 */

enum MGL_COLORS {
	MGL_BLACK,						/* dark colors	*/
	MGL_BLUE,
	MGL_GREEN,
	MGL_CYAN,
	MGL_RED,
	MGL_MAGENTA,
	MGL_BROWN,
	MGL_LIGHTGRAY,
	MGL_DARKGRAY,					/* light colors	*/
	MGL_LIGHTBLUE,
	MGL_LIGHTGREEN,
	MGL_LIGHTCYAN,
	MGL_LIGHTRED,
	MGL_LIGHTMAGENTA,
	MGL_YELLOW,
	MGL_WHITE,
	};

/* Windows standard color indices for 256 color bitmaps. 8,9,246,247 are
 * reserved and you should not count on these colors always being the
 * same. For 16 color bitmaps, colors 248-255 map to colors 8-15.
 */

enum MGL_WIN_COLORS {
	MGL_WIN_BLACK 			= 0,
	MGL_WIN_DARKRED			= 1,
	MGL_WIN_DARKGREEN		= 2,
	MGL_WIN_DARKYELLOW		= 3,
	MGL_WIN_DARKBLUE		= 4,
	MGL_WIN_DARKMAGENTA		= 5,
	MGL_WIN_DARKCYAN		= 6,
	MGL_WIN_LIGHTGRAY		= 7,
	MGL_WIN_TURQUOISE		= 8,		/* Reserved; dont count on this	*/
	MGL_WIN_SKYBLUE			= 9,		/* Reserved; dont count on this	*/
	MGL_WIN_CREAM			= 246,		/* Reserved; dont count on this	*/
	MGL_WIN_MEDIUMGRAY		= 247,		/* Reserved; dont count on this	*/
	MGL_WIN_DARKGRAY		= 248,
	MGL_WIN_LIGHTRED		= 249,
	MGL_WIN_LIGHTGREEN		= 250,
	MGL_WIN_LIGHTYELLOW		= 251,
	MGL_WIN_LIGHTBLUE		= 252,
	MGL_WIN_LIGHTMAGENTA	= 253,
    MGL_WIN_LIGHTCYAN		= 254,
	MGL_WIN_WHITE 			= 255,
	};

typedef enum {
	MGL_MARKER_SQUARE,
	MGL_MARKER_CIRCLE,
	MGL_MARKER_X,
	} MGL_markerStyleType;

typedef enum {						/* Write mode operators				*/
	MGL_REPLACE_MODE,				/* Replace mode						*/
	MGL_AND_MODE,					/* AND mode							*/
	MGL_OR_MODE,					/* OR mode							*/
	MGL_XOR_MODE,					/* XOR mode							*/
	} MGL_writeModeType;

typedef enum {
	MGL_BITMAP_SOLID,
	MGL_BITMAP_OPAQUE,
	MGL_BITMAP_TRANSPARENT,
	MGL_PIXMAP,
	} MGL_fillStyleType;

typedef enum {
	MGL_LINE_PENSTYLE,				/* Line drawn in current pen style	*/
	MGL_LINE_STIPPLE,				/* Line drawn with current stipple	*/
	} MGL_lineStyleType;

typedef enum {
	MGL_CONVEX_POLYGON,				/* Monotone vertical polygon		*/
	MGL_COMPLEX_POLYGON,			/* Non-Simple polygons				*/
	MGL_AUTO_POLYGON,				/* Auto detect the polygon type		*/
	} MGL_polygonType;

/* Text manipulation defines */

typedef enum {
	MGL_LEFT_TEXT		=	0,		/* Justify from left				*/
	MGL_TOP_TEXT		=	0,		/* Justify from top					*/
	MGL_CENTER_TEXT		=	1,		/* Center the text					*/
	MGL_RIGHT_TEXT		=	2,		/* Justify from right				*/
	MGL_BOTTOM_TEXT		=	2,		/* Justify from bottom				*/
	MGL_BASELINE_TEXT	=	3,		/* Justify from the baseline		*/
	} MGL_textJustType;

typedef enum {
	MGL_LEFT_DIR		=	0,		/* Text goes to left				*/
	MGL_UP_DIR			=	1,		/* Text goes up						*/
	MGL_RIGHT_DIR		=	2,		/* Text goes right					*/
	MGL_DOWN_DIR		=	3,		/* Text goes down					*/
	} MGL_textDirType;

/* Font types */

typedef enum {
	MGL_VECTORFONT = 1,				/* Vector font						*/
	MGL_FIXEDFONT,					/* Fixed width bitmap font			*/
	MGL_PROPFONT,					/* Proportional width bitmap font	*/
	} MGL_fontType;

/* Palette rotation directions */

typedef enum {
	MGL_ROTATE_UP,					/* Rotate the palette values up		*/
	MGL_ROTATE_DOWN,				/* Rotate the palette values down	*/
	} MGL_palRotateType;

/* Border drawing routine styles */

typedef enum {
	MGL_BDR_INSET,					/* Interior is inset into screen	*/
	MGL_BDR_OUTSET,					/* Interior is outset from screen	*/
	MGL_BDR_OUTLINE,				/* Border is 3d outline				*/
	} MGL_bdrStyleType;

/* Standard display driver names 	*/

#define	MGL_VGA4NAME		"VGA4.DRV"		/* Standard VGA drivers			*/
#define	MGL_VGA8NAME		"VGA8.DRV"
#define	MGL_VGAXNAME		"VGAX.DRV"

#define	MGL_SVGA4NAME		"SVGA4.DRV"		/* Generic SuperVGA drivers		*/
#define	MGL_SVGA8NAME		"SVGA8.DRV"
#define	MGL_SVGA16NAME		"SVGA16.DRV"
#define	MGL_SVGA24NAME		"SVGA24.DRV"
#define	MGL_SVGA32NAME		"SVGA32.DRV"

#define	MGL_LINEAR8NAME		"LINEAR8.DRV"	/* Linear framebuffer drivers	*/
#define	MGL_LINEAR16NAME	"LINEAR16.DRV"
#define	MGL_LINEAR24NAME	"LINEAR24.DRV"
#define	MGL_LINEAR32NAME	"LINEAR32.DRV"

#define	MGL_ACCEL8NAME		"ACCEL8.DRV"	/* VBE/AF Accelerated drivers	*/
#define	MGL_ACCEL16NAME		"ACCEL16.DRV"
#define	MGL_ACCEL24NAME		"ACCEL24.DRV"
#define	MGL_ACCEL32NAME		"ACCEL32.DRV"

#define	MGL_DDRAW8NAME		"DDRAW8.DRV"	/* DirectDraw drivers			*/
#define	MGL_DDRAW16NAME		"DDRAW16.DRV"
#define	MGL_DDRAW24NAME		"DDRAW24.DRV"
#define	MGL_DDRAW32NAME		"DDRAW32.DRV"

/* Standard memory driver names 	*/

#define	MGL_PACKED1NAME		"PACK1.DRV"
#define	MGL_PACKED4NAME		"PACK4.DRV"
#define	MGL_PACKED8NAME		"PACK8.DRV"
#define	MGL_PACKED16NAME	"PACK16.DRV"
#define	MGL_PACKED24NAME	"PACK24.DRV"
#define	MGL_PACKED32NAME	"PACK32.DRV"

/* Standard bitmap names	*/

#define	MGL_EMPTY_FILL	_MGL_getEmptyPat()
#define	MGL_GRAY_FILL	_MGL_getGrayPat()
#define	MGL_SOLID_FILL	_MGL_getSolidPat()

/* Event message masks for keyDown events */

#define	EVT_ASCIIMASK	0x00FF	/* Ascii code of key pressed			*/
#define	EVT_SCANMASK	0xFF00	/* Scan code of key pressed				*/
#define	EVT_COUNTMASK	0x7FFF0000L		/* Count for KEYREPEAT's		*/

#define	EVT_asciiCode(m)	( (uchar) (m & EVT_ASCIIMASK) )
#define	EVT_scanCode(m)		( (uchar) ( (m & EVT_SCANMASK) >> 8 ) )
#define	EVT_repeatCount(m)	( (short) ( (m & EVT_COUNTMASK) >> 16 ) )

/* Event message masks for mouse events */

#define	EVT_LEFTBMASK	0x0001	/* Left button is bit 0					*/
#define	EVT_RIGHTBMASK	0x0004	/* Right button is bit 1				*/
#define	EVT_BOTHBMASK	0x0005	/* Both left and right together			*/
#define	EVT_ALLBMASK	0x0005	/* All buttons pressed					*/

/* Modifier masks */

#define	EVT_LEFTBUT		0x0001	/* Set if left button was down			*/
#define	EVT_RIGHTBUT	0x0002	/* Set if right button was down			*/
#define	EVT_RIGHTSHIFT	0x0008	/* Set if right shift down				*/
#define	EVT_LEFTSHIFT	0x0010	/* Set if left shift down				*/
#define	EVT_CTRLSTATE	0x0020	/* Set if ctrl key down					*/
#define	EVT_ALTSTATE	0x0040	/* Set if alt key down					*/
#define	EVT_LEFTCTRL	0x0080	/* Set if left ctrl key down			*/
#define	EVT_LEFTALT		0x0100	/* Set if left alt key down				*/
#define	EVT_SHIFTKEY	0x0018	/* Any shift key						*/

/* Event codes */

#define	EVT_NULLEVT		0x0000	/* A null event							*/
#define	EVT_KEYDOWN		0x0001	/* Key down event						*/
#define	EVT_KEYREPEAT	0x0002	/* Key repeat event						*/
#define	EVT_KEYUP		0x0004	/* Key up event							*/
#define	EVT_MOUSEDOWN	0x0008	/* Mouse down event						*/
#define	EVT_MOUSEUP		0x0010	/* Mouse up event						*/
#define	EVT_MOUSEMOVE	0x0020	/* Mouse movement event					*/
#define	EVT_TIMERTICK	0x0040	/* Timer tick event						*/
#define	EVT_USEREVT		0x0080	/* First user event						*/

/* Event code masks */

#define	EVT_KEYEVT		(EVT_KEYDOWN | EVT_KEYREPEAT | EVT_KEYUP)
#define	EVT_MOUSEEVT	(EVT_MOUSEDOWN | EVT_MOUSEUP | EVT_MOUSEMOVE)
#define	EVT_MOUSECLICK	(EVT_MOUSEDOWN | EVT_MOUSEUP)
#define	EVT_EVERYEVT	0xFFFF

/* Suspend Application callback type codes. This callback is called
 * when the user presses one of the corresponding keys indicating that
 * they wish to change the active application. The MGL will catch these
 * events and if you have registered a callback, will call the callback to
 * save the state of the application so that it can be properly restored
 * when the user switches back to your application. The MGL takes care of
 * all the details about saving and restoring the state of the hardware,
 * and all your application needs to do is save its own state so that you can
 * re-draw the application screen upon re-activation.
 *
 * NOTE: Your application suspend callback may get called twice with the
 *		 MGL_DEACTIVATE flag in order to test whether the switch should
 *		 occur (under both DirectDraw and WinDirect fullscreen modes).
 *
 * NOTE: When your callback is called with the MGL_DEACTIVATE flag, you
 *		 cannot assume that you have access to the display memory surfaces
 *		 as they may have been lost by the time your callback has been called. 
 */

#define	MGL_DEACTIVATE	0x0001	/* Application losing active focus		*/
#define	MGL_REACTIVATE	0x0002	/* Application regaining active focus	*/

/* Return codes from the suspend application callback. The normal value
 * to be returned is MGL_SUSPEND_APP and this will cause the app to be
 * suspended while back in GDI mode until the app is re-activated again
 * by the user.
 *
 * MGL_NO_DEACTIVATE signals to WinDirect that the application does not want
 * to allow switching to occur, and the switch request will be ignored and
 * the app will remain in fullscreen mode.
 *
 * MGL_NO_SUSPEND_APP can be used to tell WinDirect to switch back to the
 * desktop, but not to suspend the application. This must be used with
 * care as the suspend application callback is then responsible for setting
 * a flag in the application that will stop the application from doing any
 * rendering to the framebuffer while the application is in GDI mode. This
 * return value is useful for games that need to maintain network
 * connectivity while the user has temporarily switched back to GDI mode.
 */

#define	MGL_NO_DEACTIVATE	0	/* Dont allow app to be deactivated 	*/
#define	MGL_SUSPEND_APP		1	/* Suspend application until restored	*/
#define	MGL_NO_SUSPEND_APP	2	/* Dont suspend, but allow switch		*/

/* Here we define the structures used to represent points and rectangles */

typedef struct {
	m_int	x,y;
	} point_t;

typedef	struct {
	m_int	left;
	m_int	top;
	m_int	right;
	m_int	bottom;
	} rect_t;

/* All colors are represented as longs by the library. This allows
 * code to work correctly with up to 24 bit color device drivers. The
 * device drivers themselves expect the color to be a color index if in
 * a color mapped mode, or a 15/16/24 bit RGB tuple in a hicolor or truecolor
 * mode. You can use the appropriate routines to pack and unpack
 * colors into the color_t format.
 */

typedef	ulong		color_t;

/* Define the value used to clear the software ZBuffer. The MGL always uses
 * a > operator for the z compare, and the smallest value is 0.
 */

#define	MGL_ZCLEARVAL	0

/* Structures for passing vertex information to polygon rendering routines.
 * All fixed point coordinates are passed in 16.16 signed fixed point
 * format, while zbuffer coordinates are passed in 4.28 signed fixed point
 * format. The sign bit is used purely for overflow and arithmetic
 * internally, and all user passed zbuffer values should be greater than
 * 0. All shaded rendering routines either take a color index in 8.16 fixed
 * point format (range 0-255.9) or separate RGB components in 8.16 fixed
 * point format (range 0-255.9).
 */

#ifdef	__FX_FIXED_H
#define	fix32_t			FXFixed
#else
typedef	long			fix32_t;
#endif
typedef	fix32_t			fxcolor_t;
typedef	long			zfix32_t;

typedef	struct {
	fix32_t x,y;
	} fxpoint_t;

typedef	struct {
	fxcolor_t	r,g,b;
	} fxrgb_t;

typedef struct {
	fix32_t		w,s,t;
	} fxtex_t;

typedef	struct {
	fxcolor_t	c;
	fxpoint_t   p;
	} fxpointc_t;

typedef	struct {
	fxrgb_t		c;
	fxpoint_t	p;
	} fxpointrgb_t;

typedef	struct {
	fxpoint_t	p;
	zfix32_t	z;
	} fxpointz_t;

typedef	struct {
	fxcolor_t	c;
	fxpoint_t	p;
	zfix32_t	z;
	} fxpointcz_t;

typedef	struct {
	fxrgb_t		c;
	fxpoint_t	p;
	zfix32_t	z;
	} fxpointrgbz_t;

/* Macros to convert between integer and 32 bit fixed point format */

#define	MGL_FIX_1			0x10000L
#define	MGL_FIX_2			0x20000L
#define	MGL_FIX_HALF		0x08000L
#define	MGL_TOFIX(i)		((long)(i) << 16)
#define MGL_FIXTOINT(f)		((m_int)((f) >> 16))
#define	MGL_FIXROUND(f) 	((m_int)(((f) + MGL_FIX_HALF) >> 16))

#define	MGL_ZFIX_1			0x10000000L
#define	MGL_ZFIX_HALF		0x08000000L
#define MGL_FIXTOZ(i)		((i) << 12)
#define	MGL_ZTOFIX(i)		((i) >> 12)
#define	MGL_TOZFIX(i)		((long)(i) << 28)
#define MGL_ZFIXTOINT(f)	((m_int)((f) >> 28))
#define	MGL_ZFIXROUND(f) 	((m_int)(((f) + MGL_ZFIX_HALF) >> 28))

/* Region structure */

#ifdef	BUILD_MGL
struct _span_t;
typedef	struct _span_t	span_t;
#else
typedef void		span_t;
#endif

typedef struct {
	rect_t      rect;			/* Bounding rectangle for region		*/
	span_t		*spans;			/* Start of span list for region		*/
	} region_t;

/* Palette entry structure	*/

typedef struct {
	uchar	blue;				/* Blue component of color				*/
	uchar	green;				/* Green component of color				*/
	uchar	red;				/* Blue component of color				*/
	uchar	alpha;				/* Alpha or alignment byte				*/
	} palette_t;

/* Maximum value for each palette entry component */

#define	PALMAX		255				/* Max value for palette components	*/

/* Pixel format structure */

typedef struct {
	uchar	redMask,greenMask;		/* Mask values for pixels			*/
	uchar	blueMask,rsvdMask;
	m_int	redPos,redAdjust;		/* Red position and adjustment		*/
	m_int	greenPos,greenAdjust;	/* Green position and adjustment	*/
	m_int	bluePos,blueAdjust;		/* Blue position and adjustment		*/
	m_int	rsvdPos,rsvdAdjust;		/* Reserved position and adjustment */
	} pixel_format_t;

/* Structure to hold arc coordinate information */

typedef struct {
	m_int	x,y;					/* Centre point of the arc			*/
	m_int	startX,startY;			/* Starting point on arc			*/
	m_int	endX,endY;				/* Ending point on arc				*/
	} arc_coords_t;

/* Mouse cursor structure */

typedef	struct {
	ulong		xorMask[32];
	ulong		andMask[32];
	m_int		xHotSpot;
	m_int		yHotSpot;
	} cursor_t;

/* Bitmap structure - always packed pixel DIB format */

typedef struct {
	m_int		width;				/* Width of bitmap in pixels		*/
	m_int		height;				/* Height of bitmap in pixels		*/
	m_int		bitsPerPixel;		/* Pixel width						*/
	m_int		bytesPerLine;		/* Bytes per line value for surface */
	uchar		*surface;			/* Pointer to bitmap surface		*/
	palette_t	*pal;				/* Palette (NULL if not loaded)		*/
	pixel_format_t *pf;				/* Pixel format (NULL if none)		*/

	/* ... palette, pixel format and bitmap data are store contiguously */
	} bitmap_t;

/* Icon structure - can be 32x23, 64x64 or in fact any size */

typedef struct {
	m_int		byteWidth;			/* Byte with for AND mask			*/
	uchar		*andMask;			/* Hold punch mask for icon			*/
	bitmap_t	xorMask;			/* XOR mask for the icon			*/

	/* ... AND mask and bitmap structure are stored contiguously */
	} icon_t;

/* Default cursor name */

#define	MGL_DEF_CURSOR		_MGL_getDefCursor()

/* Generic Font structure */

#define	_MGL_FNAMESIZE		58

typedef struct {
	char			name[_MGL_FNAMESIZE];/* Name of the font			*/
	short			fontType;		/* Type of font						*/
	short			maxWidth;		/* Maximum character width			*/
	short			maxKern;		/* Maximum character kern			*/
	short			fontWidth;		/* Font width						*/
	short			fontHeight;		/* Font height						*/
	short			ascent;			/* Font ascent value				*/
	short			descent;		/* Font descent value				*/
	short			leading;		/* Font leading value				*/
	} font_t;

/* Character and font metrics structure */

typedef struct {
	m_int			width;			/* Width of character or font		*/
	m_int			fontWidth;		/* Character width (tightest fit)	*/
	m_int			fontHeight;		/* Height of the font				*/
	m_int			ascent;			/* Ascent value						*/
	m_int			descent;		/* Descent value					*/
	m_int			leading;		/* Leading value					*/
	m_int			kern;			/* Kern value						*/
	} metrics_t;

/* Text settings structure */

typedef struct {
	m_int			horizJust;		/* Horizontal justfication			*/
	m_int			vertJust;		/* Vertical justification			*/
	m_int			dir;			/* Text drawing direction			*/
	m_int 			szNumerx;		/* Text x size numerator			*/
	m_int 			szNumery;		/* Text y size numerator			*/
	m_int			szDenomx;		/* Text x size denominator			*/
	m_int			szDenomy;		/* Text y size denominator			*/
	m_int			spaceExtra;		/* Space extra term					*/
	font_t 			*font;			/* Currently selected font			*/
	} text_settings_t;

/* Macros to access the (left,top) and (right,bottom) points of a
 * rectangle.
 */

#define	MGL_leftTop(r)		(((point_t *) &(r))[0])
#define	MGL_rightBottom(r)	(((point_t *) &(r))[1])

typedef uchar	pattern_t[8];
typedef color_t	pixpattern_t[8][8];

/* Attributes structure */

typedef struct {
	color_t			color;			/* Foreground color					*/
	color_t			backColor;		/* Background color					*/
	m_int			colorMode;		/* Current color mode				*/
	m_int			markerSize;		/* Size of markers in pixels		*/
	m_int			markerStyle;	/* Style of markers					*/
	color_t			markerColor;	/* Color to draw markers in			*/
	color_t			bdrBright;		/* Border bright color				*/
	color_t			bdrDark;		/* Border dark color				*/
	point_t			CP;				/* Graphics pen position			*/
	m_int			writeMode;		/* Scan conversion write mode op.	*/
	m_int			penStyle;		/* Pen style						*/
	m_int			penHeight;		/* Height of pen					*/
	m_int			penWidth;		/* Width of pen						*/
	pattern_t		penPat;			/* Pattern for pen					*/
	pixpattern_t	penPixPat;		/* Pixmap pattern for pen			*/
	m_int			lineStyle;		/* Line style						*/
	ushort			lineStipple;	/* Line stipple						*/
	m_uint			stippleCount;	/* Current line stipple count		*/
	rect_t			viewPort;		/* Viewport dimensions				*/
	point_t			viewPortOrg;	/* Logical viewport origin			*/
	rect_t			clipRect;		/* Clipping rectangle dimensions	*/
	m_int			clip;			/* Is clipping on?					*/
	m_int			polyType;		/* Polygon drawing type				*/
	text_settings_t	ts;				/* Text drawing attributes			*/
	} attributes_t;

/* Mode specific format information. This structrure can be used by
 * the device driver to build tables of values for all supported modes
 */

typedef struct {
	m_int	xRes;			/* Device x resolution - 1					*/
	m_int	yRes;			/* Device y resolution - 1					*/
	m_int	bitsPerPixel;	/* Number of bits per pixel					*/
	m_int	numberOfPlanes;	/* Number of planes in image				*/
	color_t	maxColor;		/* Maximum number of colors - 1				*/
	m_int	maxPage;		/* Maximum number of video pages - 1		*/
	m_int	bytesPerLine;	/* Number of bytes in a line				*/
	m_int	aspectRatio;	/* Mode aspect ratio (horiz/vert * 1000)	*/
	long	pageSize;		/* Number of bytes in a page				*/
	m_int	scratch1;		/* Scratch pad value 1						*/
	m_int	scratch2;		/* Scratch pad value 2						*/
	char    redMaskSize;            /* Size of direct color red mask    */
	char    redFieldPosition;       /* Bit posn of lsb of red mask      */
	char    greenMaskSize;          /* Size of direct color green mask  */
	char    greenFieldPosition;     /* Bit posn of lsb of green mask    */
	char    blueMaskSize;           /* Size of direct color blue mask   */
	char    blueFieldPosition;      /* Bit posn of lsb of blue mask     */
	char	rsvdMaskSize;			/* Size of reserved mask			*/
	char	rsvdFieldPosition;		/* Bit posn of reserved mask		*/
	} gmode_t;

/* Public Device Context Structure. The 'surface' member along with the
 * gmode_t information block, provides direct access to the active
 * display surface for user applications. The MGL virtualises the surface
 * in SuperVGA modes that dont have a real linear framebuffer.
 */

typedef struct {
	attributes_t a;			/* Active device attributes					*/
	void	*surface;		/* Pointer to active device surface			*/
	void	*zbuffer;		/* Pointer to Z-buffer if allocated			*/
	m_int	zbits;			/* Bits per zbuffer element					*/
	m_int   zwidth;			/* Width of the zbuffer in pixels			*/
	gmode_t	mi;				/* Mode specific information block			*/
	pixel_format_t pf;		/* Current pixel format for device context	*/
	color_t	*colorTab;		/* Color lookup table cache					*/
	color_t	*shadeTab;		/* Currently active shade table				*/
	m_int	bankOffset;		/* Offset of starting bank number			*/

	/* Remainder of Device Context structure is private and internal	*/
	} publicDevCtx_t;

#ifndef	BUILD_MGL
typedef publicDevCtx_t			MGLDC;
#else
struct internalDevCtx_t;
typedef struct internalDevCtx_t	MGLDC;
#endif

typedef	struct {
	ulong		which;			/* Which window for window manager code	*/
	m_uint		what;			/* Event code							*/
	ulong		when;			/* Clock ticks since midnight			*/
	m_int		where_x;		/* Mouse location 						*/
	m_int		where_y;
	ulong		message;		/* Event specific message				*/
	ulong		modifiers;		/* Modifier flags						*/
	m_int		next;   		/* Next event in queue					*/
	m_int		prev;			/* Previous event in queue				*/
	} event_t;

/* Structure containing file I/O functions allowing the user application to
 * completely replace the MGL's file I/O functions with their own. This
 * allows the app to store all MGL related files in a single large file,
 * with encryption or compression is desired. By default normal file I/O
 * functions will be used.
 */

typedef struct {
	FILE *	(*fopen)(const char *filename,const char *mode);
	int 	(*fclose)(FILE *f);
	int 	(*fseek)(FILE *f,long offset,int whence);
	long 	(*ftell)(FILE *f);
	size_t	(*fread)(void *ptr,size_t size,size_t n,FILE *f);
	size_t	(*fwrite)(const void *ptr,size_t size,size_t n,FILE *f);
	} fileio_t;

/* Define the flags for the types of direct surface access provided */

#define	MGL_NO_ACCESS		0x0		/* Surface cannot be accessed		*/
#define	MGL_VIRTUAL_ACCESS	0x1		/* Surface is virtualised 			*/
#define	MGL_LINEAR_ACCESS	0x2		/* Surface can be linearly accessed	*/
#define	MGL_SURFACE_FLAGS	0x3

/* Define the flags for the types of direct zbuffer access provided */

#define	MGL_NO_ZACCESS		0x0		/* Zbuffer cannot be accessed		*/
#define	MGL_VIRTUAL_ZACCESS	0x4		/* Zbuffer is virtualised in 		*/
#define	MGL_LINEAR_ZACCESS	0x8		/* Zbuffer can be linearly accessed	*/
#define	MGL_ZBUFFER_FLAGS	0xC

/* Define the flags for the types of hardware acceleration supported by
 * the device context. This will allow the application to tailor the use of
 * MGL functions depending upon whether specific hardware support is
 * available. Hence applications can use specialised software rendering
 * support if the desired hardware support is not available.
 *
 * NOTE: If the hardware flags are not MGL_HW_NONE, you *must* call
 *		 the MGL_beginDirectAccess() and MGL_endDirectAccess() functions
 *		 before and after any custom code that does direct framebuffer
 *		 rendering!!
 *
 *		 This is not necessary for non-accelerated device context, so you
 *		 might want to optimise these calls out if there is no hardware
 *		 acceleration support.
 */

#define	MGL_HW_NONE			0x0000	/* No hardware acceleration			*/
#define	MGL_HW_LINE			0x0010	/* Hardware line drawing			*/
#define	MGL_HW_STIPPLE_LINE	0x0020	/* Hardware stippled line drawing	*/
#define	MGL_HW_POLY			0x0040	/* Hardware polygon filling			*/
#define	MGL_HW_RECT			0x0080	/* Hardware rectangle fill			*/
#define	MGL_HW_PATT_RECT	0x0100	/* Hardware pattern rectangle fill	*/
#define	MGL_HW_CLRPATT_RECT	0x0200	/* Hardware color pattern fill		*/
#define	MGL_HW_SCR_BLT		0x0400	/* Hardware screen/screen bitBlt	*/
#define	MGL_HW_SRCTRANS_BLT	0x0800	/* Hardware source transparent blt	*/
#define	MGL_HW_DSTTRANS_BLT	0x1000	/* Hardware dest. transparent blt	*/
#define	MGL_HW_MONO_BLT		0x2000	/* Hardware monochrome blt			*/
#define	MGL_HW_CLIP			0x4000	/* Hardware clipping				*/
#define	MGL_HW_FLAGS		0xFFF0

#ifdef	__cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode	*/
#endif

/*------------------------- Function Prototypes ---------------------------*/

/*---------------------------------------------------------------------------
 * Routines bound to a specific device context. These routines all take
 * an MGLDC as a parmeter for the context to work with and hence dont work
 * with the current context. If however the context passed is the currently
 * active context, all changes to that context are reflected in the
 * currently active context as well.
 *-------------------------------------------------------------------------*/

/* Environment detection and initialisation */

m_int 	MGLAPI MGL_registerDriver(const char *name,void *driver);
void	MGLAPI MGL_unregisterAllDrivers(void);
void	MGLAPI MGL_registerAllDispDrivers(bool useLinear,bool useDirectDraw,bool useWinDirect);
void	MGLAPI MGL_registerAllMemDrivers(void);
void 	MGLAPI MGL_detectGraph(m_int *driver,m_int *mode);
uchar *	MGLAPI MGL_availableModes(void);
m_int	MGLAPI MGL_availablePages(m_int mode);
m_int	MGLAPI MGL_modeResolution(m_int mode,m_int *xRes,m_int *yRes,m_int *bitsPerPixel);
bool	MGLAPI MGL_isDisplayDC(MGLDC *dc);
bool	MGLAPI MGL_isWindowedDC(MGLDC *dc);
bool	MGLAPI MGL_isMemoryDC(MGLDC *dc);
void 	MGLAPI MGL_exit(void);
void 	MGLAPI MGL_setBufSize(unsigned size);
void	MGLAPI MGL_fatalError(const char *msg);
m_int 	MGLAPI MGL_result(void);
void	MGLAPI MGL_setResult(m_int result);
const char * MGLAPI MGL_errorMsg(m_int err);
const char * MGLAPI MGL_modeName(m_int mode);
const char * MGLAPI MGL_modeDriverName(m_int mode);
const char * MGLAPI MGL_driverName(m_int driver);
m_int	MGLAPI MGL_getDriver(MGLDC *dc);
m_int	MGLAPI MGL_getMode(MGLDC *dc);
m_int	MGLAPI MGL_surfaceAccessType(MGLDC *dc);
m_int	MGLAPI MGL_zbufferAccessType(MGLDC *dc);
long	MGLAPI MGL_getHardwareFlags(MGLDC *dc);
void 	MGLAPI MGL_defaultAttributes(MGLDC *dc);
void 	MGLAPI MGL_makeSubDC(MGLDC *dc,m_int left,m_int top,m_int right,m_int bottom);

/* Viewport and clip rectangle manipulation bound to a specific DC */

void 	MGLAPI MGL_setViewportDC(MGLDC *dc,rect_t view);
void	MGLAPI MGL_setRelViewportDC(MGLDC *dc,rect_t view);
void 	MGLAPI MGL_getViewportDC(MGLDC *dc,rect_t *view);
void 	MGLAPI MGL_setViewportOrgDC(MGLDC *dc,point_t org);
void	MGLAPI MGL_getViewportOrgDC(MGLDC *dc,point_t *org);
void	MGLAPI MGL_globalToLocalDC(MGLDC *dc,point_t *p);
void	MGLAPI MGL_localToGlobalDC(MGLDC *dc,point_t *p);
m_int 	MGLAPI MGL_maxxDC(MGLDC *dc);
m_int 	MGLAPI MGL_maxyDC(MGLDC *dc);
void 	MGLAPI MGL_setClipRectDC(MGLDC *dc,rect_t clip);
void 	MGLAPI MGL_getClipRectDC(MGLDC *dc,rect_t *clip);
void 	MGLAPI MGL_setClipModeDC(MGLDC *dc,bool mode);
bool 	MGLAPI MGL_getClipModeDC(MGLDC *dc);

/* Color and palette manipulation */

color_t	MGLAPI MGL_realColor(MGLDC *dc,m_int color);
color_t	MGLAPI MGL_rgbColor(MGLDC *dc,uchar R,uchar G,uchar B);
void 	MGLAPI MGL_setPaletteEntry(MGLDC *dc,m_int entry,uchar red,uchar green,uchar blue);
void 	MGLAPI MGL_getPaletteEntry(MGLDC *dc,m_int entry,uchar *red,uchar *green,uchar *blue);
void 	MGLAPI MGL_setPalette(MGLDC *dc,palette_t *pal,m_int numColors,m_int startIndex);
void 	MGLAPI MGL_getPalette(MGLDC *dc,palette_t *pal,m_int numColors,m_int startIndex);
void	ASMAPI MGL_rotatePalette(MGLDC *dc,m_int numColors,m_int startIndex,m_int direction);
bool	ASMAPI MGL_fadePalette(MGLDC *dc,palette_t *fullIntensity,m_int numColors,m_int startIndex,uchar intensity);
void	MGLAPI MGL_realizePalette(MGLDC *dc,m_int numColors,m_int startIndex,m_int waitVRT);
m_int	MGLAPI MGL_getPaletteSize(MGLDC *dc);
void	MGLAPI MGL_getDefaultPalette(MGLDC *dc,palette_t *pal);
void 	MGLAPI MGL_setDefaultPalette(MGLDC *dc);
#ifndef	MGL_LITE
bool	MGLAPI MGL_checkIdentityPalette(bool enable);
void	MGLAPI MGL_mapToPalette(MGLDC *dc,palette_t *pal);
#endif

/* Generic device context information and manipulation */

bool 	MGLAPI MGL_haveWidePalette(MGLDC *dc);
m_int	MGLAPI MGL_getBitsPerPixel(MGLDC *dc);
color_t	MGLAPI MGL_maxColor(MGLDC *dc);
m_int	MGLAPI MGL_maxPage(MGLDC *dc);
m_int 	MGLAPI MGL_sizex(MGLDC *dc);
m_int 	MGLAPI MGL_sizey(MGLDC *dc);
void 	MGLAPI MGL_getPixelFormat(MGLDC *dc,pixel_format_t *pf);
void	* MGLAPI MGL_computePixelAddr(MGLDC *dc,int x,int y);

/* Double buffering support */

void	MGLAPI MGL_setActivePage(MGLDC *dc,m_int page);
m_int	MGLAPI MGL_getActivePage(MGLDC *dc);
void	MGLAPI MGL_setVisualPage(MGLDC *dc,m_int page,m_int waitVRT);
m_int	MGLAPI MGL_getVisualPage(MGLDC *dc);
void	MGLAPI MGL_setDisplayStart(MGLDC *dc,m_int x,m_int y,m_int waitFlag);
void	MGLAPI MGL_getDisplayStart(MGLDC *dc,m_int *x,m_int *y);
void	MGLAPI MGL_vSync(MGLDC *dc);
bool	MGLAPI MGL_doubleBuffer(MGLDC *dc);
void	MGLAPI MGL_singleBuffer(MGLDC *dc);
void	MGLAPI MGL_swapBuffers(MGLDC *dc,m_int waitVRT);

/* Zbuffering support */

#ifdef	MGL_3D
m_int	MGLAPI MGL_getHardwareZBufferDepth(MGLDC *dc);
bool    ASMAPI MGL_zBegin(MGLDC *dc,m_int zbits);
bool	MGLAPI MGL_zShareZBuffer(MGLDC *dc,MGLDC *dcShared,m_int zbits);
#endif

/* Event handling support */

bool 	MGLAPI EVT_getNext(event_t *evt,m_uint mask);
bool	MGLAPI EVT_peekNext(event_t *evt,m_uint mask);
bool	MGLAPI EVT_post(ulong which,m_uint what,ulong message,ulong modifiers);
void	MGLAPI EVT_flush(m_uint mask);
void 	MGLAPI EVT_halt(event_t *evt,m_uint mask);
m_int	MGLAPI EVT_setTimerTick(m_int ticks);

/*---------------------------------------------------------------------------
 * Routines bound to the currently active context. All these routines work
 * with the currently active context and do not reflect any changes made
 * to the global context to the original user supplied context (because it
 * may be cached). The cached DC is automatically flushed back to the
 * original DC when a new context is enabled with MGL_makeCurrentDC().
 *
 * Before destroying a DC that is current, make sure you call
 * MGL_makeCurrentDC(NULL) first!
 *-------------------------------------------------------------------------*/

/* Routines to change the active global device context */

MGLDC *	MGLAPI MGL_makeCurrentDC(MGLDC *dc);
bool	MGLAPI MGL_isCurrentDC(MGLDC *dc);

/* Current device context information and manipulation */

m_int 	MGLAPI MGL_getAspectRatio(void);
void 	MGLAPI MGL_setAspectRatio(m_int aspectRatio);
void 	ASMAPI MGL_setColor(color_t color);
void	MGLAPI MGL_setColorRGB(uchar R,uchar G,uchar B);
void	MGLAPI MGL_setColorCI(m_int index);
color_t MGLAPI MGL_getColor(void);
void 	ASMAPI MGL_setBackColor(color_t color);
color_t MGLAPI MGL_getBackColor(void);
color_t	ASMAPI MGL_packColor(pixel_format_t *pf,uchar R,uchar G,uchar B);
void	MGLAPI MGL_unpackColor(pixel_format_t *pf,color_t color,uchar *R,uchar *G,uchar *B);
color_t	ASMAPI MGL_packColorRGB(uchar R,uchar G,uchar B);
void	MGLAPI MGL_unpackColorRGB(color_t color,uchar *R,uchar *G,uchar *B);
color_t	MGLAPI MGL_defaultColor(void);
#ifndef	MGL_LITE
void	MGLAPI MGL_setMarkerSize(m_int size);
m_int	MGLAPI MGL_getMarkerSize(void);
void	MGLAPI MGL_setMarkerStyle(m_int style);
m_int	MGLAPI MGL_getMarkerStyle(void);
void	MGLAPI MGL_setMarkerColor(color_t color);
color_t	MGLAPI MGL_getMarkerColor(void);
void	MGLAPI MGL_setBorderColors(color_t bright,color_t dark);
void	MGLAPI MGL_getBorderColors(color_t *bright,color_t *dark);
void 	ASMAPI MGL_setWriteMode(m_int mode);
m_int 	MGLAPI MGL_getWriteMode(void);
void 	ASMAPI MGL_setPenStyle(m_int style);
m_int 	MGLAPI MGL_getPenStyle(void);
void 	MGLAPI MGL_setLineStyle(m_int style);
m_int 	MGLAPI MGL_getLineStyle(void);
void 	ASMAPI MGL_setLineStipple(ushort stipple);
ushort	MGLAPI MGL_getLineStipple(void);
void 	ASMAPI MGL_setLineStippleCount(m_uint stippleCount);
m_uint	MGLAPI MGL_getLineStippleCount(void);
void 	ASMAPI MGL_setPenBitmapPattern(const pattern_t *pat);
void 	MGLAPI MGL_getPenBitmapPattern(pattern_t *pat);
void 	ASMAPI MGL_setPenPixmapPattern(const pixpattern_t *pat);
void 	MGLAPI MGL_getPenPixmapPattern(pixpattern_t *pat);
void	MGLAPI MGL_setPenSize(m_int height,m_int width);
void 	MGLAPI MGL_getPenSize(m_int *height,m_int *width);
#ifndef	MGL_LITE
void 	MGLAPI MGL_setColorMapMode(m_int mode);
m_int	MGLAPI MGL_getColorMapMode(void);
#endif
void	MGLAPI MGL_setPolygonType(m_int type);
m_int	MGLAPI MGL_getPolygonType(void);
#endif
void 	MGLAPI MGL_getAttributes(attributes_t *attr);
void 	MGLAPI MGL_restoreAttributes(attributes_t *attr);

/* Device clearing */

void	ASMAPI MGL_clearDevice(void);
void 	MGLAPI MGL_clearViewport(void);

/* Viewport and clip rectangle manipulation */

void 	MGLAPI MGL_setViewport(rect_t view);
void	MGLAPI MGL_setRelViewport(rect_t view);
void 	MGLAPI MGL_getViewport(rect_t *view);
void 	MGLAPI MGL_setViewportOrg(point_t org);
void	MGLAPI MGL_getViewportOrg(point_t *org);
void	MGLAPI MGL_globalToLocal(point_t *p);
void	MGLAPI MGL_localToGlobal(point_t *p);
m_int 	MGLAPI MGL_maxx(void);
m_int 	MGLAPI MGL_maxy(void);
void 	MGLAPI MGL_setClipRect(rect_t clip);
void 	MGLAPI MGL_getClipRect(rect_t *clip);
void 	MGLAPI MGL_setClipMode(bool mode);
bool 	MGLAPI MGL_getClipMode(void);

/* Pixel plotting */

void 	MGLAPI MGL_pixelCoord(m_int x,m_int y);
color_t	MGLAPI MGL_getPixelCoord(m_int x,m_int y);
void	ASMAPI MGL_beginPixel(void);
void 	MGLAPI MGL_pixelCoordFast(m_int x,m_int y);
color_t	MGLAPI MGL_getPixelCoordFast(m_int x,m_int y);
void	ASMAPI MGL_endPixel(void);

/* Line drawing and clipping */

void 	MGLAPI MGL_moveToCoord(m_int x,m_int y);
void 	MGLAPI MGL_moveRelCoord(m_int dx,m_int dy);
void 	MGLAPI MGL_lineToCoord(m_int x,m_int y);
void 	MGLAPI MGL_lineRelCoord(m_int dx,m_int dy);
m_int 	MGLAPI MGL_getX(void);
m_int 	MGLAPI MGL_getY(void);
void	MGLAPI MGL_getCP(point_t* CP);
void 	MGLAPI MGL_lineCoord(m_int x1,m_int y1,m_int x2,m_int y2);
void 	MGLAPI MGL_lineCoordFX(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void	MGLAPI MGL_lineCoordFast(m_int x1,m_int y1,m_int x2,m_int y2);
void	MGLAPI MGL_lineCoordFastFX(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    MGLAPI MGL_lineEngine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2,void (ASMAPI *plotPoint)(m_int x,m_int y));
bool	MGLAPI MGL_clipLineFX(fix32_t *x1,fix32_t *y1,fix32_t *x2,fix32_t *y2,fix32_t left,fix32_t top,fix32_t right,fix32_t bottom);
#ifndef	MGL_LITE
void 	ASMAPI MGL_scanLine(m_int y,m_int x1,m_int x2);
#endif

/* Routines to perform bank switching for banked framebuffers for custom
 * rendering code. The first version is callable only from assembler and
 * requires the new bank value to be passed in the DL register. The second
 * version is callable directly from C. DO NOT CALL THESE FUNCTIONS WHEN
 * RUNNING WITH A LINEAR FRAMEBUFFER!!!
 */

void	_ASMAPI SVGA_setBank(void);
void	_ASMAPI SVGA_setBankC(int bank);

/* Routines to begin/end direct framebuffer access. You must call these
 * functions is you wish to render directly to a hardware accelerated
 * device surface.
 */

void 	ASMAPI MGL_beginDirectAccess(void);
void 	ASMAPI MGL_endDirectAccess(void);

/* Routines to begin/end fast rendering of flat shaded lines, scanlines
 * and polygons.
 */

void 	ASMAPI MGL_beginDrawing(void);
void 	ASMAPI MGL_endDrawing(void);

/* Routines to begin/end fast rendering of smooth shaded lines, scanlines
 * and polygons.
 */

#ifdef	MGL_3D
void 	ASMAPI MGL_beginShadedDrawing(void);
void 	ASMAPI MGL_endShadedDrawing(void);
#endif

/* Routines to begin/end fast rendering of flat shaded, zbuffered lines and
 * polygons.
 */

#ifdef	MGL_3D
void 	ASMAPI MGL_beginZDrawing(void);
void 	ASMAPI MGL_endZDrawing(void);
#endif

/* Routines to begin/end fast rendering of smooth shaded, zbuffered lines and
 * polygons.
 */

#ifdef	MGL_3D
void 	ASMAPI MGL_beginZShadedDrawing(void);
void 	ASMAPI MGL_endZShadedDrawing(void);
#endif

/* Polygon drawing: Note that the following fast polygon routines
 * only work with convex polygons. The integer coordinate versions are
 * provided for compatibility only, and convert the coordinates to fixed
 * point and call the appropriate fixed point routines below.
 */

#ifndef	MGL_LITE
void	MGLAPI MGL_fillPolygon(m_int count,point_t *vArray,m_int xOffset,m_int yOffset);
void	MGLAPI MGL_fillPolygonFast(m_int count,point_t *vArray,m_int xOffset,m_int yOffset);
void    ASMAPI MGL_fillPolygonFX(m_int count,fxpoint_t *vArray,m_int vinc,fix32_t xOffset,fix32_t yOffset);
void    ASMAPI MGL_fillPolygonFastFX(m_int count,fxpoint_t *vArray,m_int vinc,fix32_t xOffset,fix32_t yOffset);
#endif

/* 3D rasterization routines */

#ifdef	MGL_3D
void    MGLAPI MGL_zClearCoord(m_int left,m_int top,m_int right,m_int bottom,zfix32_t clearVal);
#endif

#ifdef	MGL_FIX3D
void	ASMAPI MGL_cLineCoordFast(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
void	ASMAPI MGL_rgbLineCoordFast(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void	ASMAPI MGL_zLineCoordFast(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	ASMAPI MGL_czLineCoordFast(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	ASMAPI MGL_rgbzLineCoordFast(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t g2,fix32_t b2);

void	ASMAPI MGL_triFast(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
void	ASMAPI MGL_cTriFast(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fix32_t xOffset,fix32_t yOffset);
void	ASMAPI MGL_rgbTriFast(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fix32_t xOffset,fix32_t yOffset);
void	ASMAPI MGL_zTriFast(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	ASMAPI MGL_czTriFast(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	ASMAPI MGL_rgbzTriFast(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);

void	ASMAPI MGL_quadFast(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
void	ASMAPI MGL_cQuadFast(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset);
void	ASMAPI MGL_rgbQuadFast(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset);
void	ASMAPI MGL_zQuadFast(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	ASMAPI MGL_czQuadFast(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	ASMAPI MGL_rgbzQuadFast(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
#endif

/* Routine to set the currently active shade table. In HiColor and TrueColor
 * video modes, you must set a valid shadeTable before you call any of the
 * color index shaded rendering routines (MGL_cTri() etc). These routines
 * will interpolate an index into the current shade table rather than
 * each of the RGB color channels, and the appropriate full RGB color is
 * extracted directly from the shade table. The shade table can be any size,
 * but the application must ensure that the indices passed in are within
 * the range of the current shade table.
 */

#ifdef	MGL_3D
void	MGLAPI MGL_setShadeTable(color_t *shadeTab);
#endif

/* Polyline drawing */

#ifndef	MGL_LITE
void 	MGLAPI MGL_marker(point_t p);
void	MGLAPI MGL_polyPoint(m_int count,point_t *vArray);
void 	MGLAPI MGL_polyMarker(m_int count,point_t *vArray);
void 	MGLAPI MGL_polyLine(m_int count,point_t *vArray);
#endif

/* Rectangle drawing */

#ifndef	MGL_LITE
void 	MGLAPI MGL_rectCoord(m_int left,m_int top,m_int right,m_int bottom);
void 	MGLAPI MGL_fillRectCoord(m_int left,m_int top,m_int right,m_int bottom);
#endif

/* Scanline color scanning. Thee routines are primitive, and do not perform
 * any clipping or viewport mapping, so can be used to build you own
 * high performance floodfilling routines (see the example file ffill.c
 * for pre-built high speed floodfill routines).
 */

#ifndef	MGL_LITE
m_int	ASMAPI MGL_scanRightForColor(m_int x,m_int y,color_t color);
m_int	ASMAPI MGL_scanLeftForColor(m_int x,m_int y,color_t color);
m_int 	ASMAPI MGL_scanRightWhileColor(m_int x,m_int y,color_t color);
m_int	ASMAPI MGL_scanLeftWhileColor(m_int x,m_int y,color_t color);
#endif

/* Psuedo 3D border drawing */

#ifndef	MGL_LITE
void	MGLAPI MGL_drawBorderCoord(m_int left,m_int top,m_int right,m_int bottom,m_int style,m_int thickness);
void 	MGLAPI MGL_drawHDivider(m_int y,m_int x1,m_int x2);
void 	MGLAPI MGL_drawVDivider(m_int x,m_int y1,m_int y2);
#endif

/* Ellipse drawing */

#ifndef	MGL_LITE
void	MGLAPI MGL_ellipseArc(rect_t extentRect,m_int startAngle,m_int endAngle);
void	MGLAPI MGL_ellipseArcCoord(m_int x,m_int y,m_int xradius,m_int yradius,m_int startAngle,m_int endAngle);
void	ASMAPI MGL_getArcCoords(arc_coords_t *coords);
void	MGLAPI MGL_ellipse(rect_t extentRect);
void	MGLAPI MGL_ellipseCoord(m_int x,m_int y,m_int xradius,m_int yradius);
void	MGLAPI MGL_fillEllipseArc(rect_t extentRect,m_int startAngle,m_int endAngle);
void	MGLAPI MGL_fillEllipseArcCoord(m_int x,m_int y,m_int xradius,m_int yradius,m_int startAngle,m_int endAngle);
void	MGLAPI MGL_fillEllipse(rect_t extentRect);
void	MGLAPI MGL_fillEllipseCoord(m_int x,m_int y,m_int xradius,m_int yradius);
void	MGLAPI MGL_ellipseEngine(rect_t extentRect,void (ASMAPI *setup)(m_int topY,m_int botY,m_int left,m_int right),void (ASMAPI *set4pixels)(bool inc_x,bool inc_y,bool region1),void (ASMAPI *finished)(void));
void 	MGLAPI MGL_ellipseArcEngine(rect_t extentRect,m_int startAngle,m_int endAngle,arc_coords_t *ac,void (ASMAPI *plotPoint)(m_int x,m_int y));
#endif

/* Text attribute manipulation */

#ifndef	MGL_LITE
void 	MGLAPI MGL_setTextJustify(m_int horiz,m_int vert);
void 	MGLAPI MGL_getTextJustify(m_int *horiz,m_int *vert);
void 	MGLAPI MGL_setTextDirection(m_int direction);
m_int   MGLAPI MGL_getTextDirection(void);
void	MGLAPI MGL_setTextSize(m_int numerx,m_int denomx,m_int numery,m_int denomy);
void	MGLAPI MGL_getTextSize(m_int *numerx,m_int *denomx,m_int *numery,m_int *denomy);
void	MGLAPI MGL_setSpaceExtra(m_int extra);
m_int	MGLAPI MGL_getSpaceExtra(void);
void	MGLAPI MGL_setTextSettings(text_settings_t *settings);
void	MGLAPI MGL_getTextSettings(text_settings_t *settings);
m_int 	MGLAPI MGL_textHeight(void);
m_int 	MGLAPI MGL_textWidth(const char *str);
void	MGLAPI MGL_textBounds(m_int x,m_int y,const char *str,rect_t *bounds);
m_int	MGLAPI MGL_charWidth(char ch);
void	MGLAPI MGL_getFontMetrics(metrics_t *metrics);
void	MGLAPI MGL_getCharMetrics(char ch,metrics_t *metrics);
m_int	MGLAPI MGL_maxCharWidth(void);
void	MGLAPI MGL_underScoreLocation(m_int *x,m_int *y,const char *str);
#endif

/* Text drawing */

#ifndef	MGL_LITE
void 	MGLAPI MGL_drawStr(const char *str);
void	MGLAPI MGL_drawStrXY(m_int x,m_int y,const char *str);
bool	MGLAPI MGL_useFont(font_t *font);
font_t 	* MGLAPI MGL_getFont(void);
bool	MGLAPI MGL_vecFontEngine(m_int x,m_int y,const char *str,void (ASMAPI *move)(m_int x,m_int y),void (ASMAPI *draw)(m_int x,m_int y));
#endif

/* BitBlt support */

void 	MGLAPI MGL_bitBltCoord(MGLDC *dst,MGLDC *src,m_int left,m_int top,m_int right,m_int bottom,m_int dstLeft,m_int dstTop,m_int op);
void 	MGLAPI MGL_stretchBltCoord(MGLDC *dst,MGLDC *src,m_int left,m_int top,m_int right,m_int bottom,m_int dstLeft,m_int dstTop,m_int dstRight,m_int dstBottom);
#ifndef	MGL_LITE
void 	MGLAPI MGL_getDivotCoord(MGLDC *dc,m_int left,m_int top,m_int right,m_int bottom,void *divot);
void 	MGLAPI MGL_putDivot(MGLDC *dc,void *divot);
long 	MGLAPI MGL_divotSizeCoord(MGLDC *dc,m_int left,m_int top,m_int right,m_int bottom);
void 	MGLAPI MGL_putMonoImage(MGLDC *dc,m_int x,m_int y,m_int byteWidth,m_int height,void *image);
void	MGLAPI MGL_putBitmap(MGLDC *dc,m_int x,m_int y,const bitmap_t *bitmap,m_int op);
void	MGLAPI MGL_putBitmapSection(MGLDC *dc,m_int left,m_int top,m_int right,m_int bottom,m_int dstLeft,m_int dstTop,const bitmap_t *bitmap,m_int op);
void	MGLAPI MGL_putBitmapTransparent(MGLDC *dc,m_int x,m_int y,const bitmap_t *bitmap,color_t transparent,bool sourceTrans);
void	MGLAPI MGL_putBitmapTransparentSection(MGLDC *dc,m_int left,m_int top,m_int right,m_int bottom,m_int dstLeft,m_int dstTop,const bitmap_t *bitmap,color_t transparent,bool sourceTrans);
void	MGLAPI MGL_putBitmapMask(MGLDC *dc,m_int x,m_int y,const bitmap_t *mask,color_t color);
void	MGLAPI MGL_stretchBitmap(MGLDC *dc,m_int left,m_int top,m_int right,m_int bottom,const bitmap_t *bitmap);
void	MGLAPI MGL_putIcon(MGLDC *dc,m_int x,m_int y,const icon_t *icon);
void 	MGLAPI MGL_transBltCoord(MGLDC *dst,MGLDC *src,m_int left,m_int top,m_int right,m_int bottom,m_int dstLeft,m_int dstTop,color_t transparent,bool sourceTrans);
#endif

/* Linear offscreen DC BitBlt support */

#ifndef	MGL_LITE
void 	MGLAPI MGL_bitBltLinCoord(MGLDC *dst,MGLDC *src,ulong srcOfs,m_int dstLeft,m_int dstTop,m_int dstRight,m_int dstBottom,m_int op);
void 	MGLAPI MGL_transBltLinCoord(MGLDC *dst,MGLDC *src,ulong srcOfs,m_int dstLeft,m_int dstTop,m_int dstRight,m_int dstBottom,color_t transparent,bool sourceTrans);
#endif

/* Monochrome bitmap manipulation */

#ifndef	MGL_LITE
void	MGLAPI MGL_drawGlyph(font_t *g,m_int x,m_int y,uchar glyph);
m_int	MGLAPI MGL_getGlyphWidth(font_t *font,uchar glyph);
m_int	MGLAPI MGL_getGlyphHeight(font_t *font);
void 	MGLAPI MGL_rotateGlyph(uchar *dst,uchar *src,m_int *byteWidth,m_int *height,m_int rotation);
void 	MGLAPI MGL_mirrorGlyph(uchar *dst,uchar *src,m_int byteWidth,m_int height);
#endif

/* Region management */

#ifndef	MGL_LITE
region_t * MGLAPI MGL_newRegion(void);
region_t * MGLAPI MGL_copyRegion(const region_t *s);
void	MGLAPI MGL_clearRegion(region_t *r);
void 	MGLAPI MGL_freeRegion(region_t *r);
void 	MGLAPI MGL_drawRegion(m_int x,m_int y,const region_t *r);
#endif

/* Region generation primitives */

#ifndef	MGL_LITE
region_t * MGLAPI MGL_rgnLineCoord(m_int x1,m_int y1,m_int x2,m_int y2,const region_t *pen);
region_t * MGLAPI MGL_rgnLineCoordFX(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2,const region_t *pen);
/*region_t * MGLAPI MGL_rgnPolygon(m_int count,point_t *vArray);*/
/*region_t * MGLAPI MGL_rgnPolygonFast(m_int count,point_t *vArray);*/
region_t * MGLAPI MGL_rgnSolidRectCoord(m_int left,m_int top,m_int right,m_int bottom);
region_t * MGLAPI MGL_rgnEllipse(rect_t extentRect,const region_t *pen);
region_t * MGLAPI MGL_rgnEllipseArc(rect_t extentRect,m_int startAngle,m_int endAngle,const region_t *pen);
void	MGLAPI MGL_rgnGetArcCoords(arc_coords_t *coords);
region_t * MGLAPI MGL_rgnSolidEllipse(rect_t extentRect);
region_t * MGLAPI MGL_rgnSolidEllipseArc(rect_t extentRect,m_int startAngle,m_int endAngle);
#endif

/* Region alegbra */

#ifndef	MGL_LITE
region_t * MGLAPI MGL_sectRegion(const region_t *r1,const region_t *r2);
region_t * MGLAPI MGL_sectRegionRect(const region_t *r1,const rect_t *r2);
bool	MGLAPI MGL_unionRegion(region_t *r1,const region_t *r2);
bool	MGLAPI MGL_unionRegionRect(region_t *r1,const rect_t *r2);
bool 	MGLAPI MGL_unionRegionOfs(region_t *r1,const region_t *r2,m_int xOffset,m_int yOffset);
bool	MGLAPI MGL_diffRegion(region_t *r1,const region_t *r2);
bool	MGLAPI MGL_diffRegionRect(region_t *r1,const rect_t *r2);
void	MGLAPI MGL_optimizeRegion(region_t *r);
void 	MGLAPI MGL_offsetRegion(region_t *r,m_int dx,m_int dy);
bool	MGLAPI MGL_emptyRegion(const region_t *r);
bool	MGLAPI MGL_equalRegion(const region_t *r1,const region_t *r2);
bool	MGLAPI MGL_ptInRegionCoord(m_int x,m_int y,const region_t *r);
#endif

/* Region traversal */

#ifndef	MGL_LITE
typedef void (ASMAPI *rgncallback_t)(const rect_t *r);

void 	MGLAPI MGL_traverseRegion(region_t *rgn,rgncallback_t doRect);
#endif

/* RGB to 8 bit halftone dithering routines */

#ifndef	MGL_LITE
void 	MGLAPI MGL_getHalfTonePalette(palette_t *pal);
uchar	MGLAPI MGL_halfTonePixel(m_int x,m_int y,uchar R,uchar G,uchar B);
#endif

/* Resource loading/unloading */

font_t * MGLAPI MGL_loadFont(const char *fontname);
bool	MGLAPI MGL_availableFont(const char *fontname);
void	MGLAPI MGL_unloadFont(font_t *font);
cursor_t * MGLAPI MGL_loadCursor(const char *cursorName);
bool	MGLAPI MGL_availableCursor(const char *cursorName);
void 	MGLAPI MGL_unloadCursor(cursor_t *cursor);
#ifndef	MGL_LITE
icon_t * MGLAPI MGL_loadIcon(const char *iconName,bool loadPalette);
bool	MGLAPI MGL_availableIcon(const char *iconName);
void	MGLAPI MGL_unloadIcon(icon_t *icon);
#endif

/* Windows BMP bitmap loading/unloading/saving */

#ifndef	MGL_LITE
bitmap_t * MGLAPI MGL_loadBitmap(const char *bitmapName,bool loadPalette);
bool	MGLAPI MGL_availableBitmap(const char *bitmapName);
void	MGLAPI MGL_unloadBitmap(bitmap_t *bitmap);
bool	MGLAPI MGL_getBitmapSize(const char *bitmapName,m_int *width,m_int *height,m_int *bitsPerPixel,pixel_format_t *pf);
bool	MGLAPI MGL_loadBitmapIntoDC(MGLDC *dc,const char *bitmapName,m_int dstLeft,m_int dstTop,bool loadPalette);
bool	MGLAPI MGL_saveBitmapFromDC(MGLDC *dc,const char *bitmapName,m_int left,m_int top,m_int right,m_int bottom);
bitmap_t * MGLAPI MGL_getBitmapFromDC(MGLDC *dc,m_int left,m_int top,m_int right,m_int bottom,bool savePalette);
bitmap_t * MGLAPI MGL_buildMonoMask(bitmap_t *bitmap,color_t transparent);
#endif

/* PCX bitmap loading/unloading/saving (1/4/8 bpp only) */

#ifndef	MGL_LITE
bitmap_t * MGLAPI MGL_loadPCX(const char *bitmapName,bool loadPalette);
bool	MGLAPI MGL_availablePCX(const char *bitmapName);
bool	MGLAPI MGL_getPCXSize(const char *bitmapName,m_int *width,m_int *height,m_int *bitsPerPixel);
bool	MGLAPI MGL_loadPCXIntoDC(MGLDC *dc,const char *bitmapName,m_int dstLeft,m_int dstTop,bool loadPalette);
bool	MGLAPI MGL_savePCXFromDC(MGLDC *dc,const char *bitmapName,m_int left,m_int top,m_int right,m_int bottom);
#endif

/* Random number generation routines for shorts and longs with full range */

void    ASMAPI MGL_srand(m_uint seed);
ushort	ASMAPI MGL_random(ushort max);
ulong	ASMAPI MGL_randoml(ulong max);

/* Mouse support */

bool	MGLAPI MS_available(void);
void	MGLAPI MS_show(void);
void	MGLAPI MS_hide(void);
void	MGLAPI MS_obscure(void);
void	MGLAPI MS_setCursor(cursor_t *curs);
void	MGLAPI MS_setCursorColor(color_t color);
void 	MGLAPI MS_moveTo(m_int x,m_int y);
void	MGLAPI MS_getPos(m_int *x,m_int *y);
void	MGLAPI MS_drawCursor(void);

/* Rectangle and Point manipulation */

rect_t 	MGLAPI MGL_defRect(m_int left,m_int top,m_int right,m_int bottom);
rect_t 	MGLAPI MGL_defRectPt(point_t leftTop,point_t rightBottom);
bool 	MGLAPI MGL_sectRect(rect_t s1,rect_t s2,rect_t *d);
bool 	MGLAPI MGL_sectRectCoord(m_int left1,m_int top1,m_int right1,m_int bottom1,m_int left2,m_int top2,m_int right2,m_int bottom2,rect_t *d);
void 	MGLAPI MGL_unionRect(rect_t s1,rect_t s2,rect_t *d);
void 	MGLAPI MGL_unionRectCoord(m_int left1,m_int top1,m_int right1,m_int bottom1,m_int left2,m_int top2,m_int right2,m_int bottom2,rect_t *d);

/* Built-in patterns and mouse cursor */

#ifndef	MGL_LITE
pattern_t	* MGLAPI _MGL_getEmptyPat(void);
pattern_t   * MGLAPI _MGL_getGrayPat(void);
pattern_t   * MGLAPI _MGL_getSolidPat(void);
#endif
cursor_t    * MGLAPI _MGL_getDefCursor(void);

/* Fixed point multiplication/divide routines */

#if defined(__WATCOMC__) && defined(__386__)

/* For Watcom C++ we can use special inline assembler code that is much
 * faster than calling the 386 assembler functions. Currently this is the
 * the only compiler that will allow inline assembler to be expanded
 * directly as inline functions.
 */

fix32_t	MGL_FixMul(fix32_t a,fix32_t b);
#pragma aux MGL_FixMul =			\
	"imul	edx"					\
	"add	eax,8000h"				\
	"adc	edx,0"					\
	"shrd	eax,edx,16"				\
	parm [eax] [edx]				\
	value [eax]						\
	modify exact [eax edx];

fix32_t	MGL_FixDiv(fix32_t a,fix32_t b);
#pragma aux MGL_FixDiv =			\
	"xor	eax,eax"				\
	"shrd	eax,edx,16"				\
	"sar	edx,16"					\
	"idiv	ebx"              		\
	parm [edx] [ebx] 				\
	value [eax]             		\
	modify exact [eax edx];

fix32_t	MGL_FixMulDiv(fix32_t a,fix32_t b,fix32_t c);
#pragma aux MGL_FixMulDiv =			\
	"imul	ebx"					\
	"idiv	ecx"              		\
	parm [eax] [ebx] [ecx] 			\
	value [eax]             		\
	modify exact [eax edx];

m_int	MGL_backfacing(fix32_t dx1,fix32_t dy1,fix32_t dx2,fix32_t dy2);
#pragma aux MGL_backfacing =		\
	"imul	ebx"					\
	"mov	ebx,eax"				\
	"mov	ecx,edx"				\
	"mov	eax,esi"				\
	"imul	edi"					\
	"sub	eax,ebx"				\
	"mov	eax,1"					\
	"sbb	edx,ecx"				\
	"jns	@@Backfacing"			\
	"xor	eax,eax"				\
	"@@Backfacing:"					\
	parm [eax] [esi] [edi] [ebx]	\
	value [eax]             		\
	modify exact [eax ecx edx];

void MGL_memcpy(void *dst,void *src,m_int n);
#pragma aux MGL_memcpy =            \
	"mov	eax,ecx"				\
	"shr    ecx,2"                  \
	"rep    movsd"                  \
	"mov	cl,al"					\
	"and	cl,3"					\
	"rep	movsb"					\
	parm [edi] [esi] [ecx]			\
	modify exact [eax ecx esi edi];

#else

fix32_t	ASMAPI MGL_FixMul(fix32_t a,fix32_t b);
fix32_t	ASMAPI MGL_FixDiv(fix32_t a,fix32_t b);
fix32_t	ASMAPI MGL_FixMulDiv(fix32_t a,fix32_t b,fix32_t c);
m_int	ASMAPI MGL_backfacing(fix32_t dx1,fix32_t dy1,fix32_t dx2,fix32_t dy2);
void 	ASMAPI MGL_memcpy(void *dst,void *src,m_int n);

#endif

/* The following are special memcpy routines that properly handler reading
 * and writing to virtual linear buffer memory by forcing the proper
 * alignment. Note that the copy is extended to use a DWORD copy of speed.
 */

void 	ASMAPI MGL_memcpyVIRTSRC(void *dst,void *src,m_int n);
void 	ASMAPI MGL_memcpyVIRTDST(void *dst,void *src,m_int n);

/* Function to find an MGL system file's full pathname */

bool	MGLAPI _MGL_findFile(char *validpath,const char *dir, const char *filename, const char *mode);

/* Override the internal MGL file I/O functions */

void	MGLAPI MGL_setFileIO(fileio_t *fio);

/* The following dummy symbols are used to link in driver files to be used. A
 * driver is not active until it is linked in with the MGL_registerDriver
 * call. Because we dont export globals in DLLs, we provide functions to
 * get the address of the drivers. However for a static link library we
 * need to use globals so that if the driver data is unreferenced, it will
 * not be linked in with the code. 
 */

#ifndef	BUILD_MGL
#if	defined(MGL_DLL) && !defined(BUILD_MGLDLL)
void * MGLAPI VGA4_getDriverAddr(void);
void * MGLAPI VGAX_getDriverAddr(void);
void * MGLAPI SVGA4_getDriverAddr(void);
void * MGLAPI SVGA8_getDriverAddr(void);
void * MGLAPI SVGA16_getDriverAddr(void);
void * MGLAPI SVGA24_getDriverAddr(void);
void * MGLAPI SVGA32_getDriverAddr(void);
#if !defined(__16BIT__)
void * MGLAPI VGA8_getDriverAddr(void);
void * MGLAPI LINEAR8_getDriverAddr(void);
void * MGLAPI LINEAR16_getDriverAddr(void);
void * MGLAPI LINEAR24_getDriverAddr(void);
void * MGLAPI LINEAR32_getDriverAddr(void);
void * MGLAPI ACCEL8_getDriverAddr(void);
void * MGLAPI ACCEL16_getDriverAddr(void);
void * MGLAPI ACCEL24_getDriverAddr(void);
void * MGLAPI ACCEL32_getDriverAddr(void);
#if defined(MGLWIN) || defined(__WINDOWS__)
void * MGLAPI DDRAW8_getDriverAddr(void);
void * MGLAPI DDRAW16_getDriverAddr(void);
void * MGLAPI DDRAW24_getDriverAddr(void);
void * MGLAPI DDRAW32_getDriverAddr(void);
#endif
#endif
void * MGLAPI PACKED1_getDriverAddr(void);
void * MGLAPI PACKED4_getDriverAddr(void);
void * MGLAPI PACKED8_getDriverAddr(void);
void * MGLAPI PACKED16_getDriverAddr(void);
void * MGLAPI PACKED24_getDriverAddr(void);
void * MGLAPI PACKED32_getDriverAddr(void);
#define VGA4_driver			VGA4_getDriverAddr()
#define VGAX_driver			VGAX_getDriverAddr()
#define SVGA4_driver		SVGA4_getDriverAddr()
#define SVGA8_driver		SVGA8_getDriverAddr()
#define SVGA16_driver		SVGA16_getDriverAddr()
#define SVGA24_driver		SVGA24_getDriverAddr()
#define SVGA32_driver		SVGA32_getDriverAddr()
#if !defined(__16BIT__)
#define VGA8_driver			VGA8_getDriverAddr()
#define LINEAR8_driver		LINEAR8_getDriverAddr()
#define LINEAR16_driver		LINEAR16_getDriverAddr()
#define LINEAR24_driver     LINEAR24_getDriverAddr()
#define LINEAR32_driver     LINEAR32_getDriverAddr()
#define ACCEL8_driver       ACCEL8_getDriverAddr()
#define ACCEL16_driver      ACCEL16_getDriverAddr()
#define ACCEL24_driver      ACCEL24_getDriverAddr()
#define ACCEL32_driver      ACCEL32_getDriverAddr()
#if defined(MGLWIN) || defined(__WINDOWS__)
#define DDRAW8_driver       DDRAW8_getDriverAddr()
#define DDRAW16_driver      DDRAW16_getDriverAddr()
#define DDRAW24_driver      DDRAW24_getDriverAddr()
#define DDRAW32_driver      DDRAW32_getDriverAddr()
#endif
#endif
#define PACKED1_driver      PACKED1_getDriverAddr()
#define PACKED4_driver      PACKED4_getDriverAddr()
#define PACKED8_driver      PACKED8_getDriverAddr()
#define PACKED16_driver     PACKED16_getDriverAddr()
#define PACKED24_driver     PACKED24_getDriverAddr()
#define PACKED32_driver		PACKED32_getDriverAddr()
#else
extern m_int _VARAPI VGA4_driver[];
extern m_int _VARAPI VGAX_driver[];
extern m_int _VARAPI SVGA4_driver[];
extern m_int _VARAPI SVGA8_driver[];
extern m_int _VARAPI SVGA16_driver[];
extern m_int _VARAPI SVGA24_driver[];
extern m_int _VARAPI SVGA32_driver[];
#if !defined(__16BIT__)
extern m_int _VARAPI VGA8_driver[];
extern m_int _VARAPI LINEAR8_driver[];
extern m_int _VARAPI LINEAR16_driver[];
extern m_int _VARAPI LINEAR24_driver[];
extern m_int _VARAPI LINEAR32_driver[];
extern m_int _VARAPI ACCEL8_driver[];
extern m_int _VARAPI ACCEL16_driver[];
extern m_int _VARAPI ACCEL24_driver[];
extern m_int _VARAPI ACCEL32_driver[];
#if defined(MGLWIN) || defined(__WINDOWS__)
extern m_int _VARAPI DDRAW8_driver[];
extern m_int _VARAPI DDRAW16_driver[];
extern m_int _VARAPI DDRAW24_driver[];
extern m_int _VARAPI DDRAW32_driver[];
#endif
#endif
extern m_int _VARAPI PACKED1_driver[];
extern m_int _VARAPI PACKED4_driver[];
extern m_int _VARAPI PACKED8_driver[];
extern m_int _VARAPI PACKED16_driver[];
extern m_int _VARAPI PACKED24_driver[];
extern m_int _VARAPI PACKED32_driver[];
#endif
#endif

/*---------------------------------------------------------------------------
 * Memory allocation and utility functions.
 *-------------------------------------------------------------------------*/

#ifndef	__16BIT__
#define	_HUGE
#else
#define	_HUGE	_huge
#endif

void MGL_availableMemory(ulong *physical,ulong *total);
void MGL_useLocalMalloc(void _HUGE * (*malloc)(long size),void (*free)(void _HUGE *p));
void * MGLAPI MGL_malloc(long size);
void * MGLAPI MGL_calloc(long size,long n);
void MGLAPI MGL_free(void _HUGE *p);
void MGLAPI MGL_memset(void _HUGE *s,m_int c,long n);
void MGLAPI MGL_memsetw(void _HUGE *s,m_int c,long n);
void MGLAPI MGL_memsetl(void _HUGE *s,long c,long n);

/*---------------------------------------------------------------------------
 * Set a fullscreen suspend application callback function. This is used in
 * fullscreen video modes to allow switching back to the normal operating
 * system graphical shell (such as Windows GDI, OS/2 PM etc).
 *-------------------------------------------------------------------------*/

typedef m_int (ASMAPI *MGL_suspend_cb_t)(MGLDC *dc,m_int flags);
void	MGLAPI MGL_setSuspendAppCallback(MGL_suspend_cb_t staveState);

/*---------------------------------------------------------------------------
 * Tell the MGL to use a pre-loaded ACCEL.DRV driver file. This allows
 * you to link with the SciTech WinDirect/Pro and WinDirect/Ultra device
 * support libraries and tell the MGL to use the device support drivers.
 * If the user has a real ACCEL.DRV driver file in the standard location
 * on their machine, this driver file will still be used.
 *-------------------------------------------------------------------------*/

void	MGLAPI MGL_setACCELDriver(void *driver);

/*---------------------- Inline functions as Macros -----------------------*/

#define	MGL_equalPoint(p1,p2)	((p1).x == (p2).x && (p1).y == (p2).y)

#define	MGL_equalRect(r1,r2)	((r1).left == (r2).left &&			\
								 (r1).top == (r2).top &&			\
								 (r1).right == (r2).right &&		\
								 (r1).bottom == (r2).bottom)

#define	MGL_emptyRect(r)		((r).bottom <= (r).top || 			\
								 (r).right <= (r).left)

#define	MGL_disjointRect(r1,r2)	((r1).right <= (r2).left ||			\
								 (r1).left >= (r2).right ||			\
								 (r1).bottom <= (r2).top ||			\
								 (r1).top >= (r2).bottom)

#define	MGL_sectRect(s1,s2,d)										\
   ((d)->left = MAX((s1).left,(s2).left),                           \
	(d)->right = MIN((s1).right,(s2).right),                        \
	(d)->top = MAX((s1).top,(s2).top),                              \
	(d)->bottom = MIN((s1).bottom,(s2).bottom),                     \
	!MGL_emptyRect(*d))

#define	MGL_sectRectFast(s1,s2,d)									\
	(d)->left = MAX((s1).left,(s2).left);                           \
	(d)->right = MIN((s1).right,(s2).right);                        \
	(d)->top = MAX((s1).top,(s2).top);                              \
	(d)->bottom = MIN((s1).bottom,(s2).bottom)

#define	MGL_sectRectCoord(l1,t1,r1,b1,l2,t2,r2,b2,d)				\
   ((d)->left = MAX(l1,l2),											\
	(d)->right = MIN(r1,r2),										\
	(d)->top = MAX(t1,t2),											\
	(d)->bottom = MIN(b1,b2),										\
	!MGL_emptyRect(*d))

#define	MGL_sectRectFastCoord(l1,t1,r1,b1,l2,t2,r2,b2,d)			\
	(d)->left = MAX(l1,l2);											\
	(d)->right = MIN(r1,r2);										\
	(d)->top = MAX(t1,t2);											\
	(d)->bottom = MIN(b1,b2)

#define	MGL_unionRect(s1,s2,d)										\
	(d)->left = MIN((s1).left,(s2).left);                           \
	(d)->right = MAX((s1).right,(s2).right);                        \
	(d)->top = MIN((s1).top,(s2).top);                              \
	(d)->bottom = MAX((s1).bottom,(s2).bottom)

#define MGL_unionRectCoord(l1,t1,r1,b1,l2,t2,r2,b2,d)				\
	(d)->left = MIN(l1,l2);                                 		\
	(d)->right = MAX(r1,r2);                              			\
	(d)->top = MIN(t1,t2);                                    		\
	(d)->bottom = MAX(b1,b2)

#define	MGL_offsetRect(r,dx,dy)										\
	{	(r).left += dx; (r).right += dx;							\
		(r).top += dy; (r).bottom += dy; }

#define	MGL_insetRect(r,dx,dy)										\
	{	(r).left += dx; (r).right -= dx;							\
		(r).top += dy; (r).bottom -= dy;							\
		if (MGL_emptyRect(r))										\
			(r).left = (r).right = (r).top = (r).bottom = 0; }

#define	MGL_ptInRect(p,r)		((p).x >= (r).left &&				\
								 (p).x < (r).right &&				\
								 (p).y >= (r).top &&				\
								 (p).y < (r).bottom)

#define	MGL_ptInRectCoord(x,y,r)	((x) >= (r).left &&				\
									 (x) < (r).right &&				\
									 (y) >= (r).top &&				\
									 (y) < (r).bottom)

#define	MGL_ptInRegion(p,r)		MGL_ptInRegionCoord((p).x,(p).y,r)

#define	MGL_pixel(p)			MGL_pixelCoord((p).x,(p).y)
#define	MGL_getPixel(p)			MGL_getPixelCoord((p).x,(p).y)
#define	MGL_pixelFast(p)		MGL_pixelCoordFast((p).x,(p).y)
#define	MGL_getPixelFast(p)		MGL_getPixelCoordFast((p).x,(p).y)
#define	MGL_moveTo(p)			MGL_moveToCoord((p).x,(p).y)
#define MGL_moveRel(p)			MGL_moveRelCoord((p).x,(p).y)
#define	MGL_line(p1,p2)			MGL_lineCoord((p1).x,(p1).y,(p2).x,(p2).y)
#define	MGL_lineFast(p1,p2)		MGL_lineCoordFast((p1).x,(p1).y,(p2).x,(p2).y)
#define	MGL_lineFX(p1,p2)		MGL_lineCoordFX((p1).x,(p1).y,(p2).x,(p2).y)
#define	MGL_lineFastFX(p1,p2)	MGL_lineCoordFastFX((p1).x,(p1).y,(p2).x,(p2).y)
#define	MGL_cLineFast(p1,p2)	MGL_cLineCoordFast((p1).p.x,(p1).p.y,(p1).c,(p2).p.x,(p2).p.y,(p2).c)
#define	MGL_rgbLineFast(p1,p2)	MGL_rgbLineCoordFast((p1).p.x,(p1).p.y,(p1).c.r,(p1).c.g,(p1).c.b,(p2).p.x,(p2).p.y,(p2).c.r,(p2).c.g,(p2).c.b)
#define	MGL_zLineFast(p1,p2)	MGL_zLineCoordFast((p1).p.x,(p1).p.y,(p1).z,(p2).p.x,(p2).p.y,(p2).z)
#define	MGL_czLineFast(p1,p2)	MGL_czLineCoordFast((p1).p.x,(p1).p.y,(p1).z,(p1).c,(p2).p.x,(p2).p.y,(p2).z,(p2).c)
#define	MGL_rgbzLineFast(p1,p2)	MGL_rgbzLineCoordFast((p1).p.x,(p1).p.y,(p1).z,(p1).c.r,(p1).c.g,(p1).c.b,(p2).p.x,(p2).p.y,(p2).z,(p2).c.r,(p2).c.g,(p2).c.b)

#define	MGL_zClearPt(lt,rb,z)	MGL_zClearCoord((lt).x,(lt).y,	\
									(rb).x,(rb).y,z)
#define	MGL_zClear(r,z)			MGL_zClearCoord((r).left,(r).top,	\
									(r).right,(r).bottom,z)
#define	MGL_lineTo(p)			MGL_lineToCoord((p).x,(p).y)
#define	MGL_lineRel(p)			MGL_lineRelCoord((p).x,(p).y);
#define	MGL_rectPt(lt,rb)		MGL_rectCoord((lt).x,(lt).y,(rb).x,(rb).y)
#define	MGL_rect(r)				MGL_rectCoord((r).left,(r).top,			\
									(r).right,(r).bottom)
#define	MGL_drawBorder(r,s,t)	MGL_drawBorderCoord((r).left,(r).top,	\
									(r).right,(r).bottom,(s),(t))
#define	MGL_fillRectPt(lt,rb)	MGL_fillRectCoord((lt).x,(lt).y,	\
									(rb).x,(rb).y)
#define	MGL_fillRect(r)			MGL_fillRectCoord((r).left,(r).top,	\
									(r).right,(r).bottom)
#define	MGL_bitBlt(d,s,r,dl,dt,op) 	MGL_bitBltCoord((d),(s),(r).left,		\
									(r).top,(r).right,(r).bottom,dl,dt,op)
#define	MGL_bitBltLin(d,s,so,r,op) MGL_bitBltLinCoord((d),(s),so,			\
									(r).left,(r).top,(r).right,(r).bottom,op)
#define	MGL_stretchBlt(d,s,sr,dr) 	MGL_stretchBltCoord((d),(s),(sr).left,		\
									(sr).top,(sr).right,(sr).bottom,	\
									(dr).left,(dr).top,(dr).right,(dr).bottom)
#define	MGL_transBlt(d,s,r,dl,dt,c,st)	MGL_transBltCoord((d),(s),(r).left,		\
									(r).top,(r).right,(r).bottom,dl,dt,c,st)
#define	MGL_transBltLin(d,s,so,r,c,st) MGL_transBltLinCoord((d),(s),so,	\
									(r).left,(r).top,(r).right,(r).bottom,c,st)
#define	MGL_getDivot(dc,r,divot) MGL_getDivotCoord(dc,(r).left,(r).top,	\
									(r).right,(r).bottom,divot)
#define	MGL_divotSize(dc,r)		MGL_divotSizeCoord(dc,(r).left,(r).top,\
									(r).right,(r).bottom)
#define	MGL_isSimpleRegion(r)	(((region_t*)(r))->spans == NULL)
#define	MGL_rgnLine(p1,p2,p)	MGL_rgnLineCoord((p1).x,(p1).y,(p2).x,(p2).y,p)
#define	MGL_rgnLineFX(p1,p2,p)	MGL_rgnLineCoordFX((p1).x,(p1).y,(p2).x,(p2).y,p)
#define	MGL_rgnSolidRectPt(lt,rb)	MGL_rgnSolidRectCoord((lt).x,(lt).y,	\
									(rb).x,(rb).y)
#define	MGL_rgnSolidRect(r)			MGL_rgnSolidRectCoord((r).left,(r).top,	\
									(r).right,(r).bottom)

/* Fast color packing/unpacking routines implemented as macros */

#define	MGL_packColorFast(pf,R,G,B)													\
 ((ulong)(((uchar)(R) >> (pf)->redAdjust) & (pf)->redMask) << (pf)->redPos)			\
 | ((ulong)(((uchar)(G) >> (pf)->greenAdjust) & (pf)->greenMask) << (pf)->greenPos)	\
 | ((ulong)(((uchar)(B) >> (pf)->blueAdjust) & (pf)->blueMask) << (pf)->bluePos)

#define	MGL_unpackColorFast(pf,c,R,G,B)												\
{																					\
 (R) = (uchar)((((ulong)(c) >> (pf)->redPos) & (pf)->redMask) << (pf)->redAdjust);		\
 (G) = (uchar)((((ulong)(c) >> (pf)->greenPos) & (pf)->greenMask) << (pf)->greenAdjust);\
 (B) = (uchar)((((ulong)(c) >> (pf)->bluePos) & (pf)->blueMask) << (pf)->blueAdjust);	\
}

/* Macros to access the RGB components in a packed 24 bit RGB tuple */

#define	MGL_rgbRed(c)	(((uchar*)&(c))[2])
#define	MGL_rgbGreen(c)	(((uchar*)&(c))[1])
#define	MGL_rgbBlue(c)	(((uchar*)&(c))[0])

/* Fast 24 bit color packing/unpacking routines implemented as macros */

#define	MGL_packColorRGBFast(R,G,B)											\
 (((ulong)((uchar)(R)) << 16) | ((ulong)((uchar)(G)) << 8) | (uchar)(B))

#define	MGL_packColorRGBFast2(c,R,G,B)										\
{																			\
 MGL_rgbRed(c) = (uchar)(R);												\
 MGL_rgbGreen(c) = (uchar)(G);												\
 MGL_rgbBlue(c) = (uchar)(B);												\
}

#define	MGL_unpackColorRGBFast(c,R,G,B)										\
{																			\
 (R) = MGL_rgbRed(c);														\
 (G) = MGL_rgbGreen(c);														\
 (B) = MGL_rgbBlue(c);														\
}

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/

#include "mglrect.hpp"	/* Include C++ point/rectangle classes			*/

#endif	/* __cplusplus */

/* Include appropriate platform specific bindings */

#if defined(MGLWIN) || defined(__WINDOWS__)
#include "mglwin.h"
#elif defined(MGLPM) || defined(__OS2__)
/*#include "mglpm.h"*/
#elif defined(MGLX) || defined(__UNIX__)
/*#include "mglx.h"*/
#else
#include "mgldos.h"
#endif

#pragma pack()				/* Return to default packing				*/

#endif	/* __MGRAPH_H */
