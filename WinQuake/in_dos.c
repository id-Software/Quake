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
// in_mouse.c -- dos mouse code

#include "quakedef.h"
#include "dosisms.h"

#define AUX_FLAG_FREELOOK	0x00000001

typedef struct
{
	long    interruptVector;
	char    deviceName[16];
	long    numAxes;
	long	numButtons;
	long	flags;
	
	vec3_t  viewangles;

// intended velocities
	float   forwardmove;
	float   sidemove;
	float   upmove;

	long	buttons;
} externControl_t;

/*
#define AUX_FLAG_FORCEFREELOOK	0x00000001	// r/o
#define AUX_FLAG_EXTENDED		0x00000002	// r/o
#define AUX_FLAG_RUN			0x00000004	// w/o
#define AUX_FLAG_STRAFE			0x00000008	// w/o
#define AUX_FLAG_FREELOOK		0x00000010	// w/o

#define AUX_MAP_UNDEFINED	0
#define AUX_MAP_PITCH		1
#define AUX_MAP_YAW			2
#define AUX_MAP_ROLL		3
#define AUX_MAP_FORWARD		4
#define AUX_MAP_SIDE		5
#define AUX_MAP_UP			6

typedef struct
{
	long    interruptVector;
	// r/o
	char    deviceName[16];
	// r/o
	long    numAxes;
			// r/o	1-6
	long	numButtons;			// r/o	0-32
	long	flags;				// see above
	byte	axisMapping[6];		// w/o	default = p,y,r,f,s,u
	float	axisValue[6];		// r/w
	float	sensitivity[6];		// w/o	default = 1.0
	long	buttons;			// r/o
	float	last_frame_time;	// w/o
} externControl_t;
*/

cvar_t	m_filter = {"m_filter","1"};

qboolean	mouse_avail;
int		mouse_buttons;
int		mouse_oldbuttonstate;
int		mouse_buttonstate;
float	mouse_x, mouse_y;
float	old_mouse_x, old_mouse_y;


cvar_t	in_joystick = {"joystick","1"};
cvar_t	joy_numbuttons = {"joybuttons","4", true};

qboolean	joy_avail;
int		joy_oldbuttonstate;
int		joy_buttonstate;

int     joyxl, joyxh, joyyl, joyyh; 
int		joystickx, joysticky;

qboolean		need_center;

qboolean		extern_avail;
int				extern_buttons;
int				extern_oldbuttonstate;
int				extern_buttonstate;
cvar_t	aux_look = {"auxlook","1", true};
externControl_t	*extern_control;
void IN_StartupExternal (void);
void IN_ExternalMove (usercmd_t *cmd);

void IN_StartupJoystick (void);
qboolean IN_ReadJoystick (void);


void Toggle_AuxLook_f (void)
{
	if (aux_look.value)
		Cvar_Set ("auxlook","0");
	else
		Cvar_Set ("auxlook","1");
}


void Force_CenterView_f (void)
{
	cl.viewangles[PITCH] = 0;
}


/*
===========
IN_StartupMouse
===========
*/
void IN_StartupMouse (void)
{
	if ( COM_CheckParm ("-nomouse") ) 
		return; 
 
// check for mouse
	regs.x.ax = 0;
	dos_int86(0x33);
	mouse_avail = regs.x.ax;
	if (!mouse_avail)
	{
		Con_Printf ("No mouse found\n");
		return;
	}
	
	mouse_buttons = regs.x.bx;
	if (mouse_buttons > 3)
		mouse_buttons = 3;
	Con_Printf("%d-button mouse available\n", mouse_buttons);
}

/*
===========
IN_Init
===========
*/
void IN_Init (void)
{
	int i;

	Cvar_RegisterVariable (&m_filter);
	Cvar_RegisterVariable (&in_joystick);
	Cvar_RegisterVariable (&joy_numbuttons);
	Cvar_RegisterVariable (&aux_look);
	Cmd_AddCommand ("toggle_auxlook", Toggle_AuxLook_f);
	Cmd_AddCommand ("force_centerview", Force_CenterView_f);

	IN_StartupMouse ();
	IN_StartupJoystick ();

	i = COM_CheckParm ("-control");
	if (i)
	{
		extern_control = real2ptr(Q_atoi (com_argv[i+1]));
		IN_StartupExternal ();
	}
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown (void)
{

}


/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
	int		i;

	if (mouse_avail)
	{
		regs.x.ax = 3;		// read buttons
		dos_int86(0x33);
		mouse_buttonstate = regs.x.bx;
	
	// perform button actions
		for (i=0 ; i<mouse_buttons ; i++)
		{
			if ( (mouse_buttonstate & (1<<i)) &&
			!(mouse_oldbuttonstate & (1<<i)) )
			{
				Key_Event (K_MOUSE1 + i, true);
			}
			if ( !(mouse_buttonstate & (1<<i)) &&
			(mouse_oldbuttonstate & (1<<i)) )
			{
				Key_Event (K_MOUSE1 + i, false);
			}
		}	
		
		mouse_oldbuttonstate = mouse_buttonstate;
	}
	
	if (joy_avail)
	{
		joy_buttonstate = ((dos_inportb(0x201) >> 4)&15)^15;
	// perform button actions
		for (i=0 ; i<joy_numbuttons.value ; i++)
		{
			if ( (joy_buttonstate & (1<<i)) &&
			!(joy_oldbuttonstate & (1<<i)) )
			{
				Key_Event (K_JOY1 + i, true);
			}
			if ( !(joy_buttonstate & (1<<i)) &&
			(joy_oldbuttonstate & (1<<i)) )
			{
				Key_Event (K_JOY1 + i, false);
			}
		}
		
		joy_oldbuttonstate = joy_buttonstate;
	}

	if (extern_avail)
	{
		extern_buttonstate = extern_control->buttons;
	
	// perform button actions
		for (i=0 ; i<extern_buttons ; i++)
		{
			if ( (extern_buttonstate & (1<<i)) &&
			!(extern_oldbuttonstate & (1<<i)) )
			{
				Key_Event (K_AUX1 + i, true);
			}
			if ( !(extern_buttonstate & (1<<i)) &&
			(extern_oldbuttonstate & (1<<i)) )
			{
				Key_Event (K_AUX1 + i, false);
			}
		}	
		
		extern_oldbuttonstate = extern_buttonstate;
	}
	
}


/*
===========
IN_Move
===========
*/
void IN_MouseMove (usercmd_t *cmd)
{
	int		mx, my;

	if (!mouse_avail)
		return;

	regs.x.ax = 11;		// read move
	dos_int86(0x33);
	mx = (short)regs.x.cx;
	my = (short)regs.x.dx;
	
	if (m_filter.value)
	{
		mouse_x = (mx + old_mouse_x) * 0.5;
		mouse_y = (my + old_mouse_y) * 0.5;
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}
	old_mouse_x = mx;
	old_mouse_y = my;

	mouse_x *= sensitivity.value;
	mouse_y *= sensitivity.value;

// add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
		cmd->sidemove += m_side.value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;
	
	if (in_mlook.state & 1)
		V_StopPitchDrift ();
		
	if ( (in_mlook.state & 1) && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch.value * mouse_y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	}
	else
	{
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}
}

/*
===========
IN_JoyMove
===========
*/
void IN_JoyMove (usercmd_t *cmd)
{
	float	speed, aspeed;

	if (!joy_avail || !in_joystick.value) 
		return; 
 
	IN_ReadJoystick (); 
	if (joysticky > joyyh*2 || joystickx > joyxh*2)
		return;		// assume something jumped in and messed up the joystick
					// reading time (win 95)

	if (in_speed.state & 1)
		speed = cl_movespeedkey.value;
	else
		speed = 1;
	aspeed = speed*host_frametime;

	if (in_strafe.state & 1)
	{
		if (joystickx < joyxl)
			cmd->sidemove -= speed*cl_sidespeed.value; 
		else if (joystickx > joyxh) 
			cmd->sidemove += speed*cl_sidespeed.value; 
	}
	else
	{
		if (joystickx < joyxl)
			cl.viewangles[YAW] += aspeed*cl_yawspeed.value;
		else if (joystickx > joyxh) 
			cl.viewangles[YAW] -= aspeed*cl_yawspeed.value;
		cl.viewangles[YAW] = anglemod(cl.viewangles[YAW]);
	}

	if (in_mlook.state & 1)
	{
		if (m_pitch.value < 0)
			speed *= -1;
		
		if (joysticky < joyyl) 
			cl.viewangles[PITCH] += aspeed*cl_pitchspeed.value;
		else if (joysticky > joyyh) 
			cl.viewangles[PITCH] -= aspeed*cl_pitchspeed.value;
	}
	else
	{
		if (joysticky < joyyl) 
			cmd->forwardmove += speed*cl_forwardspeed.value; 
		else if (joysticky > joyyh) 
			cmd->forwardmove -= speed*cl_backspeed.value;  
	}
}

/*
===========
IN_Move
===========
*/
void IN_Move (usercmd_t *cmd)
{
	IN_MouseMove (cmd);
	IN_JoyMove (cmd);
	IN_ExternalMove (cmd);
}

/* 
============================================================================ 
 
					JOYSTICK 
 
============================================================================ 
*/



qboolean IN_ReadJoystick (void)
{
	int		b;
	int		count;

	joystickx = 0;
	joysticky = 0;

	count = 0;

	b = dos_inportb(0x201);
	dos_outportb(0x201, b);

// clear counters
	while (++count < 10000)
	{
		b = dos_inportb(0x201);

		joystickx += b&1;
		joysticky += (b&2)>>1;
		if ( !(b&3) )
			return true;
	}
	
	Con_Printf ("IN_ReadJoystick: no response\n");
	joy_avail = false;
	return false;
}

/*
=============
WaitJoyButton
=============
*/
qboolean WaitJoyButton (void) 
{ 
	int             oldbuttons, buttons; 
 
	oldbuttons = 0; 
	do 
	{
		key_count = -1;
		Sys_SendKeyEvents ();
		key_count = 0;
		if (key_lastpress == K_ESCAPE)
		{
			Con_Printf ("aborted.\n");
			return false;
		}
		key_lastpress = 0;
		SCR_UpdateScreen ();
		buttons =  ((dos_inportb(0x201) >> 4)&1)^1; 
		if (buttons != oldbuttons) 
		{ 
			oldbuttons = buttons; 
			continue; 
		}
	} while ( !buttons); 
 
	do 
	{ 
		key_count = -1;
		Sys_SendKeyEvents ();
		key_count = 0;
		if (key_lastpress == K_ESCAPE)
		{
			Con_Printf ("aborted.\n");
			return false;
		}
		key_lastpress = 0;
		SCR_UpdateScreen ();
		buttons =  ((dos_inportb(0x201) >> 4)&1)^1; 
		if (buttons != oldbuttons) 
		{ 
			oldbuttons = buttons; 
			continue; 
		} 
	} while ( buttons); 
 
	return true; 
} 
 
 
 
/* 
=============== 
IN_StartupJoystick 
=============== 
*/  
void IN_StartupJoystick (void) 
{ 
	int     centerx, centery; 
 
 	Con_Printf ("\n");

	joy_avail = false; 
	if ( COM_CheckParm ("-nojoy") ) 
		return; 
 
	if (!IN_ReadJoystick ()) 
	{ 
		joy_avail = false; 
		Con_Printf ("joystick not found\n"); 
		return; 
	} 

	Con_Printf ("joystick found\n"); 
 
	Con_Printf ("CENTER the joystick\nand press button 1 (ESC to skip):\n"); 
	if (!WaitJoyButton ()) 
		return; 
	IN_ReadJoystick (); 
	centerx = joystickx; 
	centery = joysticky; 
 
	Con_Printf ("Push the joystick to the UPPER LEFT\nand press button 1 (ESC to skip):\n"); 
	if (!WaitJoyButton ()) 
		return; 
	IN_ReadJoystick (); 
	joyxl = (centerx + joystickx)/2; 
	joyyl = (centerx + joysticky)/2; 
 
	Con_Printf ("Push the joystick to the LOWER RIGHT\nand press button 1 (ESC to skip):\n"); 
	if (!WaitJoyButton ()) 
		return; 
	IN_ReadJoystick (); 
	joyxh = (centerx + joystickx)/2; 
	joyyh = (centery + joysticky)/2; 

	joy_avail = true; 
	Con_Printf ("joystick configured.\n"); 

 	Con_Printf ("\n");
} 
 
 
/* 
============================================================================ 
 
					EXTERNAL 
 
============================================================================ 
*/


/* 
=============== 
IN_StartupExternal 
=============== 
*/  
void IN_StartupExternal (void) 
{ 
	if (extern_control->numButtons > 32)
		extern_control->numButtons = 32;

	Con_Printf("%s Initialized\n", extern_control->deviceName);
	Con_Printf("  %u axes  %u buttons\n", extern_control->numAxes, extern_control->numButtons);

	extern_avail = true;
	extern_buttons = extern_control->numButtons;
}


/*
===========
IN_ExternalMove
===========
*/
void IN_ExternalMove (usercmd_t *cmd)
{
	qboolean freelook;

	if (! extern_avail)
		return;

	extern_control->viewangles[YAW] = cl.viewangles[YAW];
	extern_control->viewangles[PITCH] = cl.viewangles[PITCH];
	extern_control->viewangles[ROLL] = cl.viewangles[ROLL];
	extern_control->forwardmove = cmd->forwardmove;
	extern_control->sidemove = cmd->sidemove;
	extern_control->upmove = cmd->upmove;

Con_DPrintf("IN:  y:%f p:%f r:%f f:%f s:%f u:%f\n", extern_control->viewangles[YAW], extern_control->viewangles[PITCH], extern_control->viewangles[ROLL], extern_control->forwardmove, extern_control->sidemove, extern_control->upmove);

	dos_int86(extern_control->interruptVector);

Con_DPrintf("OUT: y:%f p:%f r:%f f:%f s:%f u:%f\n", extern_control->viewangles[YAW], extern_control->viewangles[PITCH], extern_control->viewangles[ROLL], extern_control->forwardmove, extern_control->sidemove, extern_control->upmove);

	cl.viewangles[YAW] = extern_control->viewangles[YAW];
	cl.viewangles[PITCH] = extern_control->viewangles[PITCH];
	cl.viewangles[ROLL] = extern_control->viewangles[ROLL];
	cmd->forwardmove = extern_control->forwardmove;
	cmd->sidemove = extern_control->sidemove;
	cmd->upmove = extern_control->upmove;

	if (cl.viewangles[PITCH] > 80)
		cl.viewangles[PITCH] = 80;
	if (cl.viewangles[PITCH] < -70)
		cl.viewangles[PITCH] = -70;

	freelook = (extern_control->flags & AUX_FLAG_FREELOOK || aux_look.value || in_mlook.state & 1);

	if (freelook)
		V_StopPitchDrift ();
}

