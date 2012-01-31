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
//
// vregset.c: video register-setting interpreter
//

#include <dos.h>
#include <conio.h>

#include "quakedef.h"
#include "vregset.h"

//#define outportb	loutportb

void loutportb (int port, int val)
{
	printf ("port, val: %x %x\n", port, val);
	getch ();
}

/*
================
VideoRegisterSet
================
*/
void VideoRegisterSet (int *pregset)
{
	int		port, temp0, temp1, temp2;

	for ( ;; )
	{
		switch (*pregset++)
		{
			case VRS_END:
				return;

			case VRS_BYTE_OUT:
				port = *pregset++;
				outportb (port, *pregset++);
				break;

			case VRS_BYTE_RMW:
				port = *pregset++;
				temp0 = *pregset++;
				temp1 = *pregset++;
				temp2 = inportb (port);
				temp2 &= temp0;
				temp2 |= temp1;
				outportb (port, temp2);
				break;

			case VRS_WORD_OUT:
				port = *pregset++;
				outportb (port, *pregset & 0xFF);
				outportb (port+1, *pregset >> 8);
				pregset++;
				break;

			default:
				Sys_Error ("VideoRegisterSet: Invalid command\n");
		}
	}
}

