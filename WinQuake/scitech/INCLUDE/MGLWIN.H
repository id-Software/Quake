/****************************************************************************
*
*						MegaGraph Graphics Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   mglwin.h  $
* Version:		$Revision:   1.14  $
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for the MGLWIN bindings for MS Windows using
*				WinG in a window and WinDirect for full screen. The MGLWIN
*				binding only targets Win32 applications, so cannot be used
*				for 16 bit Windows development.
*
* $Date:   14 Mar 1997 16:09:34  $ $Author:   KendallB  $
*
****************************************************************************/

#ifndef	__MGLWIN_H
#define	__MGLWIN_H

#ifndef MGLWIN
#define	MGLWIN
#endif

/*---------------------- Macros and type definitions ----------------------*/

typedef HWND		MGL_HWND;
typedef	HDC			MGL_HDC;
typedef	HINSTANCE	MGL_HINSTANCE;

/*------------------------- Function Prototypes ---------------------------*/

#ifdef	__cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode	*/
#endif

/* Initialise the MGL for fullscreen output */

bool	MGLAPI MGL_init(m_int *driver,m_int *mode,const char *mglpath);

/* Initialise the MGL just for Windowed output, not full screen */

bool	MGLAPI MGL_initWindowed(const char *mglpath);

/* Function to register a fullscreen window with the MGL. If you wish
 * for the MGL to use your own window for fullscreen modes, you can
 * register it with this function. Note that when the MGL goes into
 * fullscreen modes, the attributes, size and position of the window are
 * modified to make it into a fullscreen Window necessary to cover the
 * entire desktop, and the state of the window will be restore to the original
 * format on return to normal GDI mode.
 *
 * Note that if you are using a common window for Windowed mode and fullscreen
 * modes of your application, you will need to ensure that certain messages
 * that you window normally handles in windowed modes are ignored when in
 * fullscreen modes.
 */

void 	MGLAPI MGL_registerFullScreenWindow(HWND hwndFullScreen);

/* Function to register a fullscreen event handling window procedure.
 * If you wish to do your own event handling, you can register your window
 * procedure with the MGL using this function and it will be called
 * when there are messages to be handled. You can still call the MGL_event()
 * functions even if you have registered an event handling procedure.
 */

void 	MGLAPI MGL_registerEventProc(WNDPROC userWndProc);

/* Change the active display mode. You must destroy all display device
 * contexts before calling this function, and re-create them again with
 * the new display mode. Does not affect any event handling hooks.
 */

bool	MGLAPI MGL_changeDisplayMode(m_int mode);

/* Obtain the handle to the MGL fullscreen window when in fullscreen modes */

MGL_HWND MGLAPI MGL_getFullScreenWindow(void);

/* Tell the MGL what your applications main window is */

void	MGLAPI MGL_setMainWindow(MGL_HWND hwnd);

/* Tell the MGL your applications instance handle (call before all funcs!) */

void	MGLAPI MGL_setAppInstance(MGL_HINSTANCE hInstApp);

/* Device context management */

MGLDC	* MGLAPI MGL_createDisplayDC(m_int numBuffers);
MGLDC	* MGLAPI MGL_createSrollingDC(m_int virtualX,m_int virtualY,m_int numBuffers);
MGLDC	* MGLAPI MGL_createOffscreenDC(void);
MGLDC	* MGLAPI MGL_createLinearOffscreenDC(void);
MGLDC	* MGLAPI MGL_createWindowedDC(MGL_HWND hwnd);
MGLDC 	* MGLAPI MGL_createMemoryDC(m_int xSize,m_int ySize,m_int bitsPerPixel,pixel_format_t *pf);
bool	MGLAPI MGL_destroyDC(MGLDC *dc);

/* Get a Windows HDC for the MGL device context. You can use this returned
 * HDC to get GDI to draw to the device context surface, such as rendering
 * and using TrueType fonts with the MGL. If a Windows compatible HDC is not
 * available, this function will return NULL.
 */

HDC		MGLAPI MGL_getWinDC(MGLDC *dc);

/* Associate a Window manager DC with the MGLDC for painting */

bool	MGLAPI MGL_setWinDC(MGLDC *dc,MGL_HDC hdc);

/* Activate the WindowDC's palette */

bool	MGLAPI MGL_activatePalette(MGLDC *dc,bool unrealize);

/* Let the MGL know when your application is being activated or deactivated.
 * This function only needs to be called when running in Windowed modes and
 * you have set the system palette to SYSPAL_NOSTATIC mode, to ensure
 * that the MGL can properly re-map your application palette when your
 * app is not active and allow Windows to re-map your bitmap colors on the
 * fly. This function should be passed a pointer to the currently active
 * MGL Windowed DC and a flag to indicate whether the app is in the background
 * or not.   
 */

void	MGLAPI MGL_appActivate(MGLDC *winDC,bool active);

/* Generic helper functions */

ulong	MGLAPI MGL_getTicks(void);
ulong	MGLAPI MGL_getTickResolution(void);
void	MGLAPI MGL_delay(m_int millseconds);
void	MGLAPI MGL_beep(m_int freq,m_int milliseconds);

/* Fullscreen specific routines */

void	MGLAPI MGL_setPaletteSnowLevel(MGLDC *dc,m_int level);
m_int	MGLAPI MGL_getPaletteSnowLevel(MGLDC *dc);

/* Determine if a specific scancode'ed key is held down (PC specific) */

bool	MGLAPI EVT_isKeyDown(uchar scanCode);

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/
#endif

#endif	/* __MGLWIN_H */
