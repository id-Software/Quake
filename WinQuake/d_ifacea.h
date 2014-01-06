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
// d_ifacea.h
//
// Include file for asm driver interface.
//

//
// !!! note that this file must match the corresponding C structures in
// d_iface.h at all times !!!
//

// !!! if this is changed, it must be changed in r_shared.h too !!!
#define ALIAS_ONSEAM				0x0020

// !!! if this is changed, it must be changed in d_iface.h too !!!
#define TURB_TEX_SIZE	64		// base turbulent texture size

// !!! if this is changed, it must be changed in d_iface.h too !!!
#define	CYCLE	128

// !!! if this is changed, it must be changed in r_shared.h too !!!
#define	MAXHEIGHT	1024

// !!! if this is changed, it must be changed in quakedef.h too !!!
#define CACHE_SIZE	32		// used to align key data structures

// particle_t structure
// !!! if this is changed, it must be changed in d_iface.h too !!!
// driver-usable fields
#define pt_org				0
#define pt_color			12
// drivers never touch the following fields
#define pt_next				16
#define pt_vel				20
#define pt_ramp				32
#define pt_die				36
#define pt_type				40
#define pt_size				44

#define PARTICLE_Z_CLIP	8.0

// finalvert_t structure
// !!! if this is changed, it must be changed in d_iface.h too !!!
#define fv_v				0	// !!! if this is moved, cases where the !!!
								// !!! address of this field is pushed in !!!
								// !!! d_polysa.s must be changed !!!
#define fv_flags			24
#define fv_reserved			28
#define fv_size				32
#define fv_shift			5


// stvert_t structure
// !!! if this is changed, it must be changed in modelgen.h too !!!
#define stv_onseam	0
#define stv_s		4
#define stv_t		8
#define stv_size	12


// trivertx_t structure
// !!! if this is changed, it must be changed in modelgen.h too !!!
#define tv_v				0
#define tv_lightnormalindex	3
#define tv_size				4

// affinetridesc_t structure
// !!! if this is changed, it must be changed in d_iface.h too !!!
#define atd_pskin			0
#define atd_pskindesc		4
#define atd_skinwidth		8
#define atd_skinheight		12
#define atd_ptriangles		16
#define atd_pfinalverts		20
#define atd_numtriangles	24
#define atd_drawtype		28
#define atd_seamfixupX16	32
#define atd_size			36

