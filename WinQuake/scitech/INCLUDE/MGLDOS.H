/****************************************************************************
*
*						MegaGraph Graphics Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   mgldos.h  $
* Version:		$Revision:   1.9  $
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for the MGLDOS binding for the MSDOS environment.
*
* $Date:   30 Jan 1997 17:31:58  $ $Author:   KendallB  $
*
****************************************************************************/

#ifndef	__MGLDOS_H
#define	__MGLDOS_H

#ifndef MGLDOS
#define	MGLDOS
#endif

/*---------------------- Macros and type definitions ----------------------*/

typedef void	*MGL_HWND;

/*------------------------- Function Prototypes ---------------------------*/

#ifdef	__cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode	*/
#endif

/* Initialise the MGL for fullscreen output */

bool	MGLAPI MGL_init(m_int *driver,m_int *mode,const char *mglpath);

/* Change the active display mode. You must destroy all display device
 * contexts before calling this function, and re-create them again with
 * the new display mode. Does not affect any event handling hooks.
 */

bool	MGLAPI MGL_changeDisplayMode(m_int mode);

/* Disable/enable event handling (call before calling MGL_init */

void 	MGLAPI MGL_useEvents(bool use);

/* Device context management */

MGLDC	* MGLAPI MGL_createDisplayDC(m_int numBuffers);
MGLDC	* MGLAPI MGL_createScrollingDC(m_int virtualX,m_int virtualY,m_int numBuffers);
MGLDC	* MGLAPI MGL_createOffscreenDC(void);
MGLDC	* MGLAPI MGL_createLinearOffscreenDC(void);
MGLDC 	* MGLAPI MGL_createMemoryDC(m_int xSize,m_int ySize,m_int bitsPerPixel,pixel_format_t *pf);
bool	MGLAPI MGL_destroyDC(MGLDC *dc);

/* Generic helper functions */

ulong	MGLAPI MGL_getTicks(void);
ulong	MGLAPI MGL_getTickResolution(void);
void	MGLAPI MGL_delay(m_int millseconds);
void	MGLAPI MGL_beep(m_int freq,m_int milliseconds);
void	MGLAPI MGL_suspend(void);
void	MGLAPI MGL_resume(void);

/* Fullscreen specific routines */

void	MGLAPI MGL_setPaletteSnowLevel(MGLDC *dc,m_int level);
m_int	MGLAPI MGL_getPaletteSnowLevel(MGLDC *dc);

/* Determine if a specific scancode'ed key is held down (PC specific) */

bool	MGLAPI EVT_isKeyDown(uchar scanCode);

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/
#endif

#endif	/* __MGLDOS_H */
