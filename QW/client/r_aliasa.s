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

Lfloat_1:	.single	1.0
Ltemp:		.long	0
Lcoords:	.long	0, 0, 0

	.text

#define fv			12+4
#define pstverts	12+8

.globl C(R_AliasTransformAndProjectFinalVerts)
C(R_AliasTransformAndProjectFinalVerts):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi
	pushl	%esi				// preserve register variables

//	int			i, temp;
//	float		lightcos, *plightnormal, zi;
//	trivertx_t	*pverts;

//	pverts = r_apverts;
	movl	C(r_apverts),%esi

//	for (i=0 ; i<r_anumverts ; i++, fv++, pverts++, pstverts++)
//	{
	movl	pstverts(%esp),%ebp
	movl	fv(%esp),%edi
	movl	C(r_anumverts),%ecx
	subl	%edx,%edx

Lloop:

//	// transform and project
//		zi = 1.0 / (DotProduct(pverts->v, aliastransform[2]) +
//				aliastransform[2][3]);
	movb	(%esi),%dl
	movb	%dl,Lcoords
	fildl	Lcoords				// v[0]
	movb	1(%esi),%dl
	movb	%dl,Lcoords+4
	fildl	Lcoords+4			// v[1] | v[0]
	movb	2(%esi),%dl	
	movb	%dl,Lcoords+8
	fildl	Lcoords+8			// v[2] | v[1] | v[0]

	fld		%st(2)				// v[0] | v[2] | v[1] | v[0]
	fmuls	C(aliastransform)+32 // accum | v[2] | v[1] | v[0]
	fld		%st(2)				// v[1] | accum | v[2] | v[1] | v[0]
	fmuls	C(aliastransform)+36 // accum2 | accum | v[2] | v[1] | v[0]
	fxch	%st(1)				// accum | accum2 | v[2] | v[1] | v[0]
	fadds	C(aliastransform)+44 // accum | accum2 | v[2] | v[1] | v[0]
	fld		%st(2)				// v[2] | accum | accum2 | v[2] | v[1] | v[0]
	fmuls	C(aliastransform)+40 // accum3 | accum | accum2 | v[2] | v[1] |
								 //  v[0]
	fxch	%st(1)				// accum | accum3 | accum2 | v[2] | v[1] | v[0]
	faddp	%st(0),%st(2)		// accum3 | accum | v[2] | v[1] | v[0]
	movb	tv_lightnormalindex(%esi),%dl
	movl	stv_s(%ebp),%eax
	movl	%eax,fv_v+8(%edi)
	faddp	%st(0),%st(1)		// z | v[2] | v[1] | v[0]

	movl	stv_t(%ebp),%eax
	movl	%eax,fv_v+12(%edi)

//	// lighting
//		plightnormal = r_avertexnormals[pverts->lightnormalindex];

	fdivrs	Lfloat_1			// zi | v[2] | v[1] | v[0]

//		fv->v[2] = pstverts->s;
//		fv->v[3] = pstverts->t;
//		fv->flags = pstverts->onseam;
	movl	stv_onseam(%ebp),%eax
	movl	%eax,fv_flags(%edi)

	movl	fv_size(%edi),%eax
	movl	stv_size(%ebp),%eax
	movl	4(%esi),%eax

	leal	(%edx,%edx,2),%eax	// index*3

	fxch	%st(3)				// v[0] | v[2] | v[1] | zi

//		lightcos = DotProduct (plightnormal, r_plightvec);
	flds	C(r_avertexnormals)(,%eax,4)
	fmuls	C(r_plightvec)
	flds	C(r_avertexnormals)+4(,%eax,4)
	fmuls	C(r_plightvec)+4
	flds	C(r_avertexnormals)+8(,%eax,4)
	fmuls	C(r_plightvec)+8
	fxch	%st(1)
	faddp	%st(0),%st(2)
	fld		%st(2)				 // v[0] | laccum | laccum2 | v[0] | v[2] |
								 //  v[1] | zi
	fmuls	C(aliastransform)+0  // xaccum | laccum | laccum2 | v[0] | v[2] |
								 //  v[1] | zi
	fxch	%st(2)				 // laccum2 | laccum | xaccum | v[0] | v[2] |
								 //  v[1] | zi
	faddp	%st(0),%st(1)		 // laccum | xaccum | v[0] | v[2] | v[1] | zi

//		temp = r_ambientlight;
//		if (lightcos < 0)
//		{
	fsts	Ltemp
	movl	C(r_ambientlight),%eax
	movb	Ltemp+3,%dl
	testb	$0x80,%dl
	jz		Lsavelight	// no need to clamp if only ambient lit, because
						//  r_ambientlight is preclamped

//			temp += (int)(r_shadelight * lightcos);
	fmuls	C(r_shadelight)
// FIXME: fast float->int conversion?
	fistpl	Ltemp
	addl	Ltemp,%eax

//		// clamp; because we limited the minimum ambient and shading light, we
//		// don't have to clamp low light, just bright
//			if (temp < 0)
//				temp = 0;
	jns		Lp1
	subl	%eax,%eax

//		}

Lp1:

//		fv->v[4] = temp;
//
//	// x, y, and z are scaled down by 1/2**31 in the transform, so 1/z is
//	// scaled up by 1/2**31, and the scaling cancels out for x and y in the
//	// projection
//		fv->v[0] = ((DotProduct(pverts->v, aliastransform[0]) +
//				aliastransform[0][3]) * zi) + aliasxcenter;
//		fv->v[1] = ((DotProduct(pverts->v, aliastransform[1]) +
//				aliastransform[1][3]) * zi) + aliasycenter;
//		fv->v[5] = zi;
	fxch	%st(1)				 // v[0] | xaccum | v[2] | v[1] | zi
	fmuls	C(aliastransform)+16 // yaccum | xaccum | v[2] | v[1] | zi
	fxch	%st(3)				 // v[1] | xaccum | v[2] | yaccum | zi
	fld		%st(0)				 // v[1] | v[1] | xaccum | v[2] | yaccum | zi
	fmuls	C(aliastransform)+4	 // xaccum2 | v[1] | xaccum | v[2] | yaccum |zi
	fxch	%st(1)				 // v[1] | xaccum2 | xaccum | v[2] | yaccum |zi
	movl	%eax,fv_v+16(%edi)
	fmuls	C(aliastransform)+20 // yaccum2 | xaccum2 | xaccum | v[2] | yaccum|
								 //  zi
	fxch	%st(2)				 // xaccum | xaccum2 | yaccum2 | v[2] | yaccum|
								 //  zi
	fadds	C(aliastransform)+12 // xaccum | xaccum2 | yaccum2 | v[2] | yaccum|
								 //  zi
	fxch	%st(4)				 // yaccum | xaccum2 | yaccum2 | v[2] | xaccum|
								 //  zi
	fadds	C(aliastransform)+28 // yaccum | xaccum2 | yaccum2 | v[2] | xaccum|
								 //  zi
	fxch	%st(3)				 // v[2] | xaccum2 | yaccum2 | yaccum | xaccum|
								 //  zi
	fld		%st(0)				 // v[2] | v[2] | xaccum2 | yaccum2 | yaccum |
								 //  xaccum | zi
	fmuls	C(aliastransform)+8	 // xaccum3 | v[2] | xaccum2 | yaccum2 |yaccum|
								 //  xaccum | zi
	fxch	%st(1)				 // v[2] | xaccum3 | xaccum2 | yaccum2 |yaccum|
								 //  xaccum | zi
	fmuls	C(aliastransform)+24 // yaccum3 | xaccum3 | xaccum2 | yaccum2 |
								 // yaccum | xaccum | zi
	fxch	%st(5)				 // xaccum | xaccum3 | xaccum2 | yaccum2 |
								 // yaccum | yaccum3 | zi
	faddp	%st(0),%st(2)		 // xaccum3 | xaccum | yaccum2 | yaccum |
								 //  yaccum3 | zi
	fxch	%st(3)				 // yaccum | xaccum | yaccum2 | xaccum3 |
								 //  yaccum3 | zi
	faddp	%st(0),%st(2)		 // xaccum | yaccum | xaccum3 | yaccum3 | zi
	addl	$(tv_size),%esi
	faddp	%st(0),%st(2)		 // yaccum | x | yaccum3 | zi
	faddp	%st(0),%st(2)		 // x | y | zi
	addl	$(stv_size),%ebp
	fmul	%st(2),%st(0)		 // x/z | y | zi
	fxch	%st(1)				 // y | x/z | zi
	fmul	%st(2),%st(0)		 // y/z | x/z | zi
	fxch	%st(1)				 // x/z | y/z | zi
	fadds	C(aliasxcenter)		 // u | y/z | zi
	fxch	%st(1)				 // y/z | u | zi
	fadds	C(aliasycenter)		 // v | u | zi
	fxch	%st(2)				 // zi | u | v
// FIXME: fast float->int conversion?
	fistpl	fv_v+20(%edi)		 // u | v
	fistpl	fv_v+0(%edi)		 // v
	fistpl	fv_v+4(%edi)

//	}

	addl	$(fv_size),%edi
	decl	%ecx
	jnz		Lloop

	popl	%esi				// restore register variables
	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	ret

Lsavelight:
	fstp	%st(0)
	jmp		Lp1

#endif	// id386

