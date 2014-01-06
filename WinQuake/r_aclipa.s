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
// r_aliasa.s
// x86 assembly-language Alias model transform and project code.
//

#include "asm_i386.h"
#include "quakeasm.h"
#include "asm_draw.h"
#include "d_ifacea.h"

#if id386

	.data
Ltemp0:	.long	0
Ltemp1:	.long	0

	.text

#define pfv0		8+4
#define pfv1		8+8
#define out			8+12

.globl C(R_Alias_clip_bottom)
C(R_Alias_clip_bottom):
	pushl	%esi
	pushl	%edi

	movl	pfv0(%esp),%esi
	movl	pfv1(%esp),%edi

	movl	C(r_refdef)+rd_aliasvrectbottom,%eax

LDoForwardOrBackward:

	movl	fv_v+4(%esi),%edx
	movl	fv_v+4(%edi),%ecx

	cmpl	%ecx,%edx
	jl		LDoForward

	movl	fv_v+4(%esi),%ecx
	movl	fv_v+4(%edi),%edx
	movl	pfv0(%esp),%edi
	movl	pfv1(%esp),%esi

LDoForward:

	subl	%edx,%ecx
	subl	%edx,%eax
	movl	%ecx,Ltemp1
	movl	%eax,Ltemp0
	fildl	Ltemp1
	fildl	Ltemp0
	movl	out(%esp),%edx
	movl	$2,%eax

	fdivp	%st(0),%st(1)					// scale

LDo3Forward:
	fildl	fv_v+0(%esi)	// fv0v0 | scale
	fildl	fv_v+0(%edi)	// fv1v0 | fv0v0 | scale
	fildl	fv_v+4(%esi)	// fv0v1 | fv1v0 | fv0v0 | scale
	fildl	fv_v+4(%edi)	// fv1v1 | fv0v1 | fv1v0 | fv0v0 | scale
	fildl	fv_v+8(%esi)	// fv0v2 | fv1v1 | fv0v1 | fv1v0 | fv0v0 | scale
	fildl	fv_v+8(%edi)	// fv1v2 | fv0v2 | fv1v1 | fv0v1 | fv1v0 | fv0v0 |
							//  scale
	fxch	%st(5)			// fv0v0 | fv0v2 | fv1v1 | fv0v1 | fv1v0 | fv1v2 |
							//  scale
	fsubr	%st(0),%st(4)	// fv0v0 | fv0v2 | fv1v1 | fv0v1 | fv1v0-fv0v0 |
							//  fv1v2 | scale
	fxch	%st(3)			// fv0v1 | fv0v2 | fv1v1 | fv0v0 | fv1v0-fv0v0 |
							//  fv1v2 | scale
	fsubr	%st(0),%st(2)	// fv0v1 | fv0v2 | fv1v1-fv0v1 | fv0v0 |
							//  fv1v0-fv0v0 | fv1v2 | scale
	fxch	%st(1)			// fv0v2 | fv0v1 | fv1v1-fv0v1 | fv0v0 |
							//  fv1v0-fv0v0 | fv1v2 | scale
	fsubr	%st(0),%st(5)	// fv0v2 | fv0v1 | fv1v1-fv0v1 | fv0v0 |
							//  fv1v0-fv0v0 | fv1v2-fv0v2 | scale
	fxch	%st(6)			// scale | fv0v1 | fv1v1-fv0v1 | fv0v0 |
							//  fv1v0-fv0v0 | fv1v2-fv0v2 | fv0v2
	fmul	%st(0),%st(4)	// scale | fv0v1 | fv1v1-fv0v1 | fv0v0 |
							//  (fv1v0-fv0v0)*scale | fv1v2-fv0v2 | fv0v2
	addl	$12,%edi
	fmul	%st(0),%st(2)	// scale | fv0v1 | (fv1v1-fv0v1)*scale | fv0v0 |
							//  (fv1v0-fv0v0)*scale | fv1v2-fv0v2 | fv0v2
	addl	$12,%esi
	addl	$12,%edx
	fmul	%st(0),%st(5)	// scale | fv0v1 | (fv1v1-fv0v1)*scale | fv0v0 |
							//  (fv1v0-fv0v0)*scale | (fv1v2-fv0v2)*scale |
							//  fv0v2
	fxch	%st(3)			// fv0v0 | fv0v1 | (fv1v1-fv0v1)*scale | scale |
							//  (fv1v0-fv0v0)*scale | (fv1v2-fv0v2)*scale |
							//  fv0v2
	faddp	%st(0),%st(4)	// fv0v1 | (fv1v1-fv0v1)*scale | scale |
							//  fv0v0+(fv1v0-fv0v0)*scale |
							//  (fv1v2-fv0v2)*scale | fv0v2
	faddp	%st(0),%st(1)	// fv0v1+(fv1v1-fv0v1)*scale | scale |
							//  fv0v0+(fv1v0-fv0v0)*scale |
							//  (fv1v2-fv0v2)*scale | fv0v2
	fxch	%st(4)			// fv0v2 | scale | fv0v0+(fv1v0-fv0v0)*scale |
							//  (fv1v2-fv0v2)*scale | fv0v1+(fv1v1-fv0v1)*scale
	faddp	%st(0),%st(3)	// scale | fv0v0+(fv1v0-fv0v0)*scale |
							//  fv0v2+(fv1v2-fv0v2)*scale |
							//  fv0v1+(fv1v1-fv0v1)*scale
	fxch	%st(1)			// fv0v0+(fv1v0-fv0v0)*scale | scale | 
							//  fv0v2+(fv1v2-fv0v2)*scale |
							//  fv0v1+(fv1v1-fv0v1)*scale
	fadds	float_point5
	fxch	%st(3)			// fv0v1+(fv1v1-fv0v1)*scale | scale | 
							//  fv0v2+(fv1v2-fv0v2)*scale |
							//  fv0v0+(fv1v0-fv0v0)*scale
	fadds	float_point5
	fxch	%st(2)			// fv0v2+(fv1v2-fv0v2)*scale | scale | 
							//  fv0v1+(fv1v1-fv0v1)*scale |
							//  fv0v0+(fv1v0-fv0v0)*scale
	fadds	float_point5
	fxch	%st(3)			// fv0v0+(fv1v0-fv0v0)*scale | scale | 
							//  fv0v1+(fv1v1-fv0v1)*scale |
							//  fv0v2+(fv1v2-fv0v2)*scale
	fistpl	fv_v+0-12(%edx)	// scale | fv0v1+(fv1v1-fv0v1)*scale |
							//  fv0v2+(fv1v2-fv0v2)*scale
	fxch	%st(1)			// fv0v1+(fv1v1-fv0v1)*scale | scale |
							//  fv0v2+(fv1v2-fv0v2)*scale | scale
	fistpl	fv_v+4-12(%edx)	// scale | fv0v2+(fv1v2-fv0v2)*scale
	fxch	%st(1)			// fv0v2+(fv1v2-fv0v2)*sc | scale
	fistpl	fv_v+8-12(%edx)	// scale

	decl	%eax
	jnz		LDo3Forward

	fstp	%st(0)

	popl	%edi
	popl	%esi

	ret


.globl C(R_Alias_clip_top)
C(R_Alias_clip_top):
	pushl	%esi
	pushl	%edi

	movl	pfv0(%esp),%esi
	movl	pfv1(%esp),%edi

	movl	C(r_refdef)+rd_aliasvrect+4,%eax
	jmp		LDoForwardOrBackward



.globl C(R_Alias_clip_right)
C(R_Alias_clip_right):
	pushl	%esi
	pushl	%edi

	movl	pfv0(%esp),%esi
	movl	pfv1(%esp),%edi

	movl	C(r_refdef)+rd_aliasvrectright,%eax

LRightLeftEntry:


	movl	fv_v+4(%esi),%edx
	movl	fv_v+4(%edi),%ecx

	cmpl	%ecx,%edx
	movl	fv_v+0(%esi),%edx

	movl	fv_v+0(%edi),%ecx
	jl		LDoForward2

	movl	fv_v+0(%esi),%ecx
	movl	fv_v+0(%edi),%edx
	movl	pfv0(%esp),%edi
	movl	pfv1(%esp),%esi

LDoForward2:

	jmp		LDoForward


.globl C(R_Alias_clip_left)
C(R_Alias_clip_left):
	pushl	%esi
	pushl	%edi

	movl	pfv0(%esp),%esi
	movl	pfv1(%esp),%edi

	movl	C(r_refdef)+rd_aliasvrect+0,%eax
	jmp		LRightLeftEntry


#endif	// id386

