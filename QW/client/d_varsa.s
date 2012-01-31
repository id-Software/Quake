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
// d_varsa.s
//

#include "asm_i386.h"
#include "quakeasm.h"
#include "asm_draw.h"
#include "d_ifacea.h"

#if	id386

	.data

//-------------------------------------------------------
// global refresh variables
//-------------------------------------------------------

// FIXME: put all refresh variables into one contiguous block. Make into one
// big structure, like cl or sv?

	.align	4
.globl	C(d_sdivzstepu)
.globl	C(d_tdivzstepu)
.globl	C(d_zistepu)
.globl	C(d_sdivzstepv)
.globl	C(d_tdivzstepv)
.globl	C(d_zistepv)
.globl	C(d_sdivzorigin)
.globl	C(d_tdivzorigin)
.globl	C(d_ziorigin)
C(d_sdivzstepu):	.single	0
C(d_tdivzstepu):	.single	0
C(d_zistepu):		.single	0
C(d_sdivzstepv):	.single	0
C(d_tdivzstepv):	.single	0
C(d_zistepv):		.single	0
C(d_sdivzorigin):	.single	0
C(d_tdivzorigin):	.single	0
C(d_ziorigin):		.single	0

.globl	C(sadjust)
.globl	C(tadjust)
.globl	C(bbextents)
.globl	C(bbextentt)
C(sadjust):			.long	0
C(tadjust):			.long	0
C(bbextents):		.long	0
C(bbextentt):		.long	0

.globl	C(cacheblock)
.globl	C(d_viewbuffer)
.globl	C(cachewidth)
.globl	C(d_pzbuffer)
.globl	C(d_zrowbytes)
.globl	C(d_zwidth)
C(cacheblock):		.long	0
C(cachewidth):		.long	0
C(d_viewbuffer):	.long	0
C(d_pzbuffer):		.long	0
C(d_zrowbytes):		.long	0
C(d_zwidth):		.long	0


//-------------------------------------------------------
// ASM-only variables
//-------------------------------------------------------
.globl	izi
izi:			.long	0

.globl	pbase, s, t, sfracf, tfracf, snext, tnext
.globl	spancountminus1, zi16stepu, sdivz16stepu, tdivz16stepu
.globl	zi8stepu, sdivz8stepu, tdivz8stepu, pz
s:				.long	0
t:				.long	0
snext:			.long	0
tnext:			.long	0
sfracf:			.long	0
tfracf:			.long	0
pbase:			.long	0
zi8stepu:		.long	0
sdivz8stepu:	.long	0
tdivz8stepu:	.long	0
zi16stepu:		.long	0
sdivz16stepu:	.long	0
tdivz16stepu:	.long	0
spancountminus1: .long	0
pz:				.long	0

.globl	izistep
izistep:				.long	0

//-------------------------------------------------------
// local variables for d_draw16.s
//-------------------------------------------------------

.globl	reciprocal_table_16, entryvec_table_16
// 1/2, 1/3, 1/4, 1/5, 1/6, 1/7, 1/8, 1/9, 1/10, 1/11, 1/12, 1/13,
// 1/14, and 1/15 in 0.32 form
reciprocal_table_16:	.long	0x40000000, 0x2aaaaaaa, 0x20000000
						.long	0x19999999, 0x15555555, 0x12492492
						.long	0x10000000, 0xe38e38e, 0xccccccc, 0xba2e8ba
						.long	0xaaaaaaa, 0x9d89d89, 0x9249249, 0x8888888

#ifndef NeXT
	.extern Entry2_16
	.extern Entry3_16
	.extern Entry4_16
	.extern Entry5_16
	.extern Entry6_16
	.extern Entry7_16
	.extern Entry8_16
	.extern Entry9_16
	.extern Entry10_16
	.extern Entry11_16
	.extern Entry12_16
	.extern Entry13_16
	.extern Entry14_16
	.extern Entry15_16
	.extern Entry16_16
#endif

entryvec_table_16:	.long	0, Entry2_16, Entry3_16, Entry4_16
					.long	Entry5_16, Entry6_16, Entry7_16, Entry8_16
					.long	Entry9_16, Entry10_16, Entry11_16, Entry12_16
					.long	Entry13_16, Entry14_16, Entry15_16, Entry16_16

//-------------------------------------------------------
// local variables for d_parta.s
//-------------------------------------------------------
.globl	DP_Count, DP_u, DP_v, DP_32768, DP_Color, DP_Pix, DP_EntryTable
DP_Count:		.long	0
DP_u:			.long	0
DP_v:			.long	0
DP_32768:		.single	32768.0
DP_Color:		.long	0
DP_Pix:			.long	0


#ifndef NeXT
	.extern DP_1x1
	.extern DP_2x2
	.extern DP_3x3
	.extern DP_4x4
#endif

DP_EntryTable:	.long	DP_1x1, DP_2x2, DP_3x3, DP_4x4

//
// advancetable is 8 bytes, but points to the middle of that range so negative
// offsets will work
//
.globl	advancetable, sstep, tstep, pspantemp, counttemp, jumptemp
advancetable:	.long	0, 0
sstep:			.long	0
tstep:			.long	0

pspantemp:		.long	0
counttemp:		.long	0
jumptemp:		.long	0

// 1/2, 1/3, 1/4, 1/5, 1/6, and 1/7 in 0.32 form
.globl	reciprocal_table, entryvec_table
reciprocal_table:	.long	0x40000000, 0x2aaaaaaa, 0x20000000
					.long	0x19999999, 0x15555555, 0x12492492

#ifndef NeXT
	.extern Entry2_8
	.extern Entry3_8
	.extern Entry4_8
	.extern Entry5_8
	.extern Entry6_8
	.extern Entry7_8
	.extern Entry8_8
#endif

entryvec_table:	.long	0, Entry2_8, Entry3_8, Entry4_8
				.long	Entry5_8, Entry6_8, Entry7_8, Entry8_8

#ifndef NeXT
	.extern Spr8Entry2_8
	.extern Spr8Entry3_8
	.extern Spr8Entry4_8
	.extern Spr8Entry5_8
	.extern Spr8Entry6_8
	.extern Spr8Entry7_8
	.extern Spr8Entry8_8
#endif
	
.globl spr8entryvec_table
spr8entryvec_table:	.long	0, Spr8Entry2_8, Spr8Entry3_8, Spr8Entry4_8
					.long	Spr8Entry5_8, Spr8Entry6_8, Spr8Entry7_8, Spr8Entry8_8

#endif	// id386

