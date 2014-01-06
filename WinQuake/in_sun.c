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
// in_sun.c -- SUN/X mouse input handler

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

#include "quakedef.h"


//
// typedefs and defines
//

#define MOUSE_SCALE		4

//
// externs
//

extern Display			*x_disp;
extern int				x_screen, x_screen_width, x_screen_height;
extern int			x_center_height, x_center_width;
extern int				x_std_event_mask;
extern Window			x_win, x_root_win;
extern qboolean			x_fullscreen;
extern qboolean			x_focus;
extern int			global_dx, global_dy;
//
// globals
//

cvar_t					_windowed_mouse = {"_windowed_mouse","1", true};
int					x_root, y_root;
int					x_root_old, y_root_old;
//
// locals
//

static int				x_mouse_num, x_mouse_denom, x_mouse_thresh;


static qboolean x_grabbed = false;

//
// IN_CenterMouse - center the mouse in the screen
//

void IN_CenterMouse( void )
{
	CheckMouseState();

	if (!x_grabbed)
		return;

	XSelectInput( x_disp, x_win, x_std_event_mask & ~PointerMotionMask );
	XWarpPointer( x_disp, None, x_root_win, 0, 0, 0, 0, x_center_width,
		      x_center_height );
	XSelectInput( x_disp, x_win, x_std_event_mask );
}

//
// Check to see if we have grabbed the mouse or not and deal with it
// appropriately
//
static void CheckMouseState(void)
{
	if (x_focus && _windowed_mouse.value && !x_grabbed) {
		x_grabbed = true;
		printf("fooling with mouse!\n");
		if (XGetPointerControl( x_disp, &x_mouse_num, &x_mouse_denom, &x_mouse_thresh ))
			printf( "XGetPointerControl failed!\n" );
		//printf( "mouse %d/%d thresh %d\n", x_mouse_num, x_mouse_denom, x_mouse_thresh );

		// make input rawer
		XAutoRepeatOff(x_disp);
		XGrabKeyboard(x_disp, x_win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
		XGrabPointer(x_disp, x_win, True, 
			     PointerMotionMask | ButtonPressMask | ButtonReleaseMask, 
			     GrabModeAsync, GrabModeAsync, None, None, CurrentTime);

//		if (XChangePointerControl( x_disp, True, True, 1, MOUSE_SCALE, x_mouse_thresh ))
//			printf( "XChangePointerControl failed!\n" );

		IN_CenterMouse();

		// safe initial values
		x_root = x_root_old = vid.width >> 1;
		y_root = y_root_old = vid.height >> 1;
	} else if (x_grabbed && (!_windowed_mouse.value || !x_focus)) {
		printf("fooling with mouse!\n");
		x_grabbed = false;
		// undo mouse warp
		if (XChangePointerControl( x_disp, True, True, x_mouse_num, x_mouse_denom, x_mouse_thresh ))
			printf( "XChangePointerControl failed!\n" );

		XUngrabPointer( x_disp, CurrentTime );
		XUngrabKeyboard( x_disp, CurrentTime );
		XAutoRepeatOn( x_disp );
	}
}


//
// IN_Init - setup mouse input
//

void IN_Init (void)
{
    if (!x_disp) Sys_Error( "X display not open!\n" );

    Cvar_RegisterVariable (&_windowed_mouse);

	// we really really want to clean these up...
    atexit( IN_Shutdown );
}

//
// IN_Shutdown - clean up mouse settings (must be done from signal handler too!)
//

void IN_Shutdown (void)
{
    if (!x_disp) return;

	// undo mouse warp
	if (XChangePointerControl( x_disp, True, True, x_mouse_num, x_mouse_denom, x_mouse_thresh ))
		printf( "XChangePointerControl failed!\n" );

	XUngrabPointer( x_disp, CurrentTime );
	XUngrabKeyboard( x_disp, CurrentTime );
	XAutoRepeatOn( x_disp );
}

//
// IN_Commands - process buttons
//

void IN_Commands (void)
{
	// done in X event handler
}

//
// IN_Move - process mouse moves
//

void
IN_Move (usercmd_t *cmd)
{
	static int last_dx, last_dy;
	static long long last_movement;
	long long now, gethrtime();

	int dx, dy;

	CheckMouseState();


	if (!x_grabbed)
		return; // no mouse movement
	

	now = gethrtime();

	dx = global_dx;
	global_dx = 0;

	dy = global_dy;
	global_dy = 0;
	
//	printf("GOT: dx %d dy %d\n", dx, dy);

	dx *= sensitivity.value;
	dy *= sensitivity.value;

//
//	implement low pass filter to smooth motion a bit
//	
	if (now - last_movement > 100000000) {
		dx = .6 * dx;
		dy = .6 * dy;
	}
	last_movement = now;

	dx = .6 * dx + .4 * last_dx;
	dy = .6 * dy + .4 * last_dy;


	last_dx = dx;
	last_dy = dy;

	if (!dx && !dy) {
		if (in_mlook.state & 1) 
			V_StopPitchDrift ();
		return;
	}
	
	// add mouse X/Y movement to cmd
	if ((in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1)))
		cmd->sidemove += m_side.value * dx;
	else 
		cl.viewangles[YAW] -= m_yaw.value * dx;

	if (in_mlook.state & 1) 
		V_StopPitchDrift ();
	    
	if ((in_mlook.state & 1) && !(in_strafe.state & 1)) {
		cl.viewangles[PITCH] += m_pitch.value * dy;
		if (cl.viewangles[PITCH] > 80) cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70) cl.viewangles[PITCH] = -70;
	}
	else {
		if ((in_strafe.state & 1) && noclip_anglehack) cmd->upmove -= m_forward.value * dy;
		else cmd->forwardmove -= m_forward.value * dy;
	}
}
