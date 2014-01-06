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
// surf16.s
// x86 assembly-language 16 bpp surface block drawing code.
//

#include "asm_i386.h"
#include "quakeasm.h"
#include "asm_draw.h"

#if id386

//----------------------------------------------------------------------
// Surface block drawer
//----------------------------------------------------------------------

	.data

k:			.long	0
loopentry:	.long	0

	.align	4
blockjumptable16:
	.long	LEnter2_16
	.long	LEnter4_16
	.long	0, LEnter8_16
	.long	0, 0, 0, LEnter16_16


	.text

	.align 4
.globl C(R_Surf16Start)
C(R_Surf16Start):

	.align 4
.globl C(R_DrawSurfaceBlock16)
C(R_DrawSurfaceBlock16):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi
	pushl	%esi				// preserve register variables
	pushl	%ebx

	movl	C(blocksize),%eax
	movl	C(prowdestbase),%edi
	movl	C(pbasesource),%esi
	movl	C(sourcesstep),%ebx
	movl	blockjumptable16-4(,%eax,2),%ecx
	movl	%eax,k
	movl	%ecx,loopentry
	movl	C(lightleft),%edx
	movl	C(lightright),%ebp

Lblockloop16:

	subl	%edx,%ebp
	movb	C(blockdivshift),%cl
	sarl	%cl,%ebp
	jns		Lp1_16
	testl	C(blockdivmask),%ebp
	jz		Lp1_16
	incl	%ebp
Lp1_16:

	subl	%eax,%eax
	subl	%ecx,%ecx	// high words must be 0 in loop for addressing

	jmp		*loopentry

	.align	4

#include "block16.h"

	movl	C(pbasesource),%esi
	movl	C(lightleft),%edx
	movl	C(lightright),%ebp
	movl	C(sourcetstep),%eax
	movl	C(lightrightstep),%ecx
	movl	C(prowdestbase),%edi

	addl	%eax,%esi
	addl	%ecx,%ebp

	movl	C(lightleftstep),%eax
	movl	C(surfrowbytes),%ecx

	addl	%eax,%edx
	addl	%ecx,%edi

	movl	%esi,C(pbasesource)
	movl	%ebp,C(lightright)
	movl	k,%eax
	movl	%edx,C(lightleft)
	decl	%eax
	movl	%edi,C(prowdestbase)
	movl	%eax,k
	jnz		Lblockloop16

	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	ret

.globl C(R_Surf16End)
C(R_Surf16End):

//----------------------------------------------------------------------
// Code patching routines
//----------------------------------------------------------------------
	.data

	.align 4
LPatchTable16:
	.long	LBPatch0-4
	.long	LBPatch1-4
	.long	LBPatch2-4
	.long	LBPatch3-4
	.long	LBPatch4-4
	.long	LBPatch5-4
	.long	LBPatch6-4
	.long	LBPatch7-4
	.long	LBPatch8-4
	.long	LBPatch9-4
	.long	LBPatch10-4
	.long	LBPatch11-4
	.long	LBPatch12-4
	.long	LBPatch13-4
	.long	LBPatch14-4
	.long	LBPatch15-4

	.text

	.align 4
.globl C(R_Surf16Patch)
C(R_Surf16Patch):
	pushl	%ebx

	movl	C(colormap),%eax
	movl	$LPatchTable16,%ebx
	movl	$16,%ecx
LPatchLoop16:
	movl	(%ebx),%edx
	addl	$4,%ebx
	movl	%eax,(%edx)
	decl	%ecx
	jnz		LPatchLoop16

	popl	%ebx

	ret


#endif	// id386
