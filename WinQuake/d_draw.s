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
// d_draw.s
// x86 assembly-language horizontal 8-bpp span-drawing code.
//

#include "asm_i386.h"
#include "quakeasm.h"
#include "asm_draw.h"
#include "d_ifacea.h"

#if	id386

//----------------------------------------------------------------------
// 8-bpp horizontal span drawing code for polygons, with no transparency.
//
// Assumes there is at least one span in pspans, and that every span
// contains at least one pixel
//----------------------------------------------------------------------

	.text

// out-of-line, rarely-needed clamping code

LClampHigh0:
	movl	C(bbextents),%esi
	jmp		LClampReentry0
LClampHighOrLow0:
	jg		LClampHigh0
	xorl	%esi,%esi
	jmp		LClampReentry0

LClampHigh1:
	movl	C(bbextentt),%edx
	jmp		LClampReentry1
LClampHighOrLow1:
	jg		LClampHigh1
	xorl	%edx,%edx
	jmp		LClampReentry1

LClampLow2:
	movl	$2048,%ebp
	jmp		LClampReentry2
LClampHigh2:
	movl	C(bbextents),%ebp
	jmp		LClampReentry2

LClampLow3:
	movl	$2048,%ecx
	jmp		LClampReentry3
LClampHigh3:
	movl	C(bbextentt),%ecx
	jmp		LClampReentry3

LClampLow4:
	movl	$2048,%eax
	jmp		LClampReentry4
LClampHigh4:
	movl	C(bbextents),%eax
	jmp		LClampReentry4

LClampLow5:
	movl	$2048,%ebx
	jmp		LClampReentry5
LClampHigh5:
	movl	C(bbextentt),%ebx
	jmp		LClampReentry5


#define pspans	4+16

	.align 4
.globl C(D_DrawSpans8)
C(D_DrawSpans8):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi
	pushl	%esi				// preserve register variables
	pushl	%ebx

//
// set up scaled-by-8 steps, for 8-long segments; also set up cacheblock
// and span list pointers
//
// TODO: any overlap from rearranging?
	flds	C(d_sdivzstepu)
	fmuls	fp_8
	movl	C(cacheblock),%edx
	flds	C(d_tdivzstepu)
	fmuls	fp_8
	movl	pspans(%esp),%ebx	// point to the first span descriptor
	flds	C(d_zistepu)
	fmuls	fp_8
	movl	%edx,pbase			// pbase = cacheblock
	fstps	zi8stepu
	fstps	tdivz8stepu
	fstps	sdivz8stepu

LSpanLoop:
//
// set up the initial s/z, t/z, and 1/z on the FP stack, and generate the
// initial s and t values
//
// FIXME: pipeline FILD?
	fildl	espan_t_v(%ebx)
	fildl	espan_t_u(%ebx)

	fld		%st(1)			// dv | du | dv
	fmuls	C(d_sdivzstepv)	// dv*d_sdivzstepv | du | dv
	fld		%st(1)			// du | dv*d_sdivzstepv | du | dv
	fmuls	C(d_sdivzstepu)	// du*d_sdivzstepu | dv*d_sdivzstepv | du | dv
	fld		%st(2)			// du | du*d_sdivzstepu | dv*d_sdivzstepv | du | dv
	fmuls	C(d_tdivzstepu)	// du*d_tdivzstepu | du*d_sdivzstepu |
							//  dv*d_sdivzstepv | du | dv
	fxch	%st(1)			// du*d_sdivzstepu | du*d_tdivzstepu |
							//  dv*d_sdivzstepv | du | dv
	faddp	%st(0),%st(2)	// du*d_tdivzstepu |
							//  du*d_sdivzstepu + dv*d_sdivzstepv | du | dv
	fxch	%st(1)			// du*d_sdivzstepu + dv*d_sdivzstepv |
							//  du*d_tdivzstepu | du | dv
	fld		%st(3)			// dv | du*d_sdivzstepu + dv*d_sdivzstepv |
							//  du*d_tdivzstepu | du | dv
	fmuls	C(d_tdivzstepv)	// dv*d_tdivzstepv |
							//  du*d_sdivzstepu + dv*d_sdivzstepv |
							//  du*d_tdivzstepu | du | dv
	fxch	%st(1)			// du*d_sdivzstepu + dv*d_sdivzstepv |
							//  dv*d_tdivzstepv | du*d_tdivzstepu | du | dv
	fadds	C(d_sdivzorigin)	// sdivz = d_sdivzorigin + dv*d_sdivzstepv +
							//  du*d_sdivzstepu; stays in %st(2) at end
	fxch	%st(4)			// dv | dv*d_tdivzstepv | du*d_tdivzstepu | du |
							//  s/z
	fmuls	C(d_zistepv)		// dv*d_zistepv | dv*d_tdivzstepv |
							//  du*d_tdivzstepu | du | s/z
	fxch	%st(1)			// dv*d_tdivzstepv |  dv*d_zistepv |
							//  du*d_tdivzstepu | du | s/z
	faddp	%st(0),%st(2)	// dv*d_zistepv |
							//  dv*d_tdivzstepv + du*d_tdivzstepu | du | s/z
	fxch	%st(2)			// du | dv*d_tdivzstepv + du*d_tdivzstepu |
							//  dv*d_zistepv | s/z
	fmuls	C(d_zistepu)		// du*d_zistepu |
							//  dv*d_tdivzstepv + du*d_tdivzstepu |
							//  dv*d_zistepv | s/z
	fxch	%st(1)			// dv*d_tdivzstepv + du*d_tdivzstepu |
							//  du*d_zistepu | dv*d_zistepv | s/z
	fadds	C(d_tdivzorigin)	// tdivz = d_tdivzorigin + dv*d_tdivzstepv +
							//  du*d_tdivzstepu; stays in %st(1) at end
	fxch	%st(2)			// dv*d_zistepv | du*d_zistepu | t/z | s/z
	faddp	%st(0),%st(1)	// dv*d_zistepv + du*d_zistepu | t/z | s/z

	flds	fp_64k			// fp_64k | dv*d_zistepv + du*d_zistepu | t/z | s/z
	fxch	%st(1)			// dv*d_zistepv + du*d_zistepu | fp_64k | t/z | s/z
	fadds	C(d_ziorigin)		// zi = d_ziorigin + dv*d_zistepv +
							//  du*d_zistepu; stays in %st(0) at end
							// 1/z | fp_64k | t/z | s/z
//
// calculate and clamp s & t
//
	fdivr	%st(0),%st(1)	// 1/z | z*64k | t/z | s/z

//
// point %edi to the first pixel in the span
//
	movl	C(d_viewbuffer),%ecx
	movl	espan_t_v(%ebx),%eax
	movl	%ebx,pspantemp	// preserve spans pointer

	movl	C(tadjust),%edx
	movl	C(sadjust),%esi
	movl	C(d_scantable)(,%eax,4),%edi	// v * screenwidth
	addl	%ecx,%edi
	movl	espan_t_u(%ebx),%ecx
	addl	%ecx,%edi				// pdest = &pdestspan[scans->u];
	movl	espan_t_count(%ebx),%ecx

//
// now start the FDIV for the end of the span
//
	cmpl	$8,%ecx
	ja		LSetupNotLast1

	decl	%ecx
	jz		LCleanup1		// if only one pixel, no need to start an FDIV
	movl	%ecx,spancountminus1

// finish up the s and t calcs
	fxch	%st(1)			// z*64k | 1/z | t/z | s/z

	fld		%st(0)			// z*64k | z*64k | 1/z | t/z | s/z
	fmul	%st(4),%st(0)	// s | z*64k | 1/z | t/z | s/z
	fxch	%st(1)			// z*64k | s | 1/z | t/z | s/z
	fmul	%st(3),%st(0)	// t | s | 1/z | t/z | s/z
	fxch	%st(1)			// s | t | 1/z | t/z | s/z
	fistpl	s				// 1/z | t | t/z | s/z
	fistpl	t				// 1/z | t/z | s/z

	fildl	spancountminus1

	flds	C(d_tdivzstepu)	// C(d_tdivzstepu) | spancountminus1
	flds	C(d_zistepu)		// C(d_zistepu) | C(d_tdivzstepu) | spancountminus1
	fmul	%st(2),%st(0)	// C(d_zistepu)*scm1 | C(d_tdivzstepu) | scm1
	fxch	%st(1)			// C(d_tdivzstepu) | C(d_zistepu)*scm1 | scm1
	fmul	%st(2),%st(0)	// C(d_tdivzstepu)*scm1 | C(d_zistepu)*scm1 | scm1
	fxch	%st(2)			// scm1 | C(d_zistepu)*scm1 | C(d_tdivzstepu)*scm1
	fmuls	C(d_sdivzstepu)	// C(d_sdivzstepu)*scm1 | C(d_zistepu)*scm1 |
							//  C(d_tdivzstepu)*scm1
	fxch	%st(1)			// C(d_zistepu)*scm1 | C(d_sdivzstepu)*scm1 |
							//  C(d_tdivzstepu)*scm1
	faddp	%st(0),%st(3)	// C(d_sdivzstepu)*scm1 | C(d_tdivzstepu)*scm1
	fxch	%st(1)			// C(d_tdivzstepu)*scm1 | C(d_sdivzstepu)*scm1
	faddp	%st(0),%st(3)	// C(d_sdivzstepu)*scm1
	faddp	%st(0),%st(3)

	flds	fp_64k
	fdiv	%st(1),%st(0)	// this is what we've gone to all this trouble to
							//  overlap
	jmp		LFDIVInFlight1

LCleanup1:
// finish up the s and t calcs
	fxch	%st(1)			// z*64k | 1/z | t/z | s/z

	fld		%st(0)			// z*64k | z*64k | 1/z | t/z | s/z
	fmul	%st(4),%st(0)	// s | z*64k | 1/z | t/z | s/z
	fxch	%st(1)			// z*64k | s | 1/z | t/z | s/z
	fmul	%st(3),%st(0)	// t | s | 1/z | t/z | s/z
	fxch	%st(1)			// s | t | 1/z | t/z | s/z
	fistpl	s				// 1/z | t | t/z | s/z
	fistpl	t				// 1/z | t/z | s/z
	jmp		LFDIVInFlight1

	.align	4
LSetupNotLast1:
// finish up the s and t calcs
	fxch	%st(1)			// z*64k | 1/z | t/z | s/z

	fld		%st(0)			// z*64k | z*64k | 1/z | t/z | s/z
	fmul	%st(4),%st(0)	// s | z*64k | 1/z | t/z | s/z
	fxch	%st(1)			// z*64k | s | 1/z | t/z | s/z
	fmul	%st(3),%st(0)	// t | s | 1/z | t/z | s/z
	fxch	%st(1)			// s | t | 1/z | t/z | s/z
	fistpl	s				// 1/z | t | t/z | s/z
	fistpl	t				// 1/z | t/z | s/z

	fadds	zi8stepu
	fxch	%st(2)
	fadds	sdivz8stepu
	fxch	%st(2)
	flds	tdivz8stepu
	faddp	%st(0),%st(2)
	flds	fp_64k
	fdiv	%st(1),%st(0)	// z = 1/1/z
							// this is what we've gone to all this trouble to
							//  overlap
LFDIVInFlight1:

	addl	s,%esi
	addl	t,%edx
	movl	C(bbextents),%ebx
	movl	C(bbextentt),%ebp
	cmpl	%ebx,%esi
	ja		LClampHighOrLow0
LClampReentry0:
	movl	%esi,s
	movl	pbase,%ebx
	shll	$16,%esi
	cmpl	%ebp,%edx
	movl	%esi,sfracf
	ja		LClampHighOrLow1
LClampReentry1:
	movl	%edx,t
	movl	s,%esi					// sfrac = scans->sfrac;
	shll	$16,%edx
	movl	t,%eax					// tfrac = scans->tfrac;
	sarl	$16,%esi
	movl	%edx,tfracf

//
// calculate the texture starting address
//
	sarl	$16,%eax
	movl	C(cachewidth),%edx
	imull	%edx,%eax				// (tfrac >> 16) * cachewidth
	addl	%ebx,%esi
	addl	%eax,%esi				// psource = pbase + (sfrac >> 16) +
									//           ((tfrac >> 16) * cachewidth);

//
// determine whether last span or not
//
	cmpl	$8,%ecx
	jna		LLastSegment

//
// not the last segment; do full 8-wide segment
//
LNotLastSegment:

//
// advance s/z, t/z, and 1/z, and calculate s & t at end of span and steps to
// get there
//

// pick up after the FDIV that was left in flight previously

	fld		%st(0)			// duplicate it
	fmul	%st(4),%st(0)	// s = s/z * z
	fxch	%st(1)
	fmul	%st(3),%st(0)	// t = t/z * z
	fxch	%st(1)
	fistpl	snext
	fistpl	tnext
	movl	snext,%eax
	movl	tnext,%edx

	movb	(%esi),%bl	// get first source texel
	subl	$8,%ecx		// count off this segments' pixels
	movl	C(sadjust),%ebp
	movl	%ecx,counttemp	// remember count of remaining pixels

	movl	C(tadjust),%ecx
	movb	%bl,(%edi)	// store first dest pixel

	addl	%eax,%ebp
	addl	%edx,%ecx

	movl	C(bbextents),%eax
	movl	C(bbextentt),%edx

	cmpl	$2048,%ebp
	jl		LClampLow2
	cmpl	%eax,%ebp
	ja		LClampHigh2
LClampReentry2:

	cmpl	$2048,%ecx
	jl		LClampLow3
	cmpl	%edx,%ecx
	ja		LClampHigh3
LClampReentry3:

	movl	%ebp,snext
	movl	%ecx,tnext

	subl	s,%ebp
	subl	t,%ecx
	
//
// set up advancetable
//
	movl	%ecx,%eax
	movl	%ebp,%edx
	sarl	$19,%eax			// tstep >>= 16;
	jz		LZero
	sarl	$19,%edx			// sstep >>= 16;
	movl	C(cachewidth),%ebx
	imull	%ebx,%eax
	jmp		LSetUp1

LZero:
	sarl	$19,%edx			// sstep >>= 16;
	movl	C(cachewidth),%ebx

LSetUp1:

	addl	%edx,%eax			// add in sstep
								// (tstep >> 16) * cachewidth + (sstep >> 16);
	movl	tfracf,%edx
	movl	%eax,advancetable+4	// advance base in t
	addl	%ebx,%eax			// ((tstep >> 16) + 1) * cachewidth +
								//  (sstep >> 16);
	shll	$13,%ebp			// left-justify sstep fractional part
	movl	sfracf,%ebx
	shll	$13,%ecx			// left-justify tstep fractional part
	movl	%eax,advancetable	// advance extra in t

	movl	%ecx,tstep
	addl	%ecx,%edx			// advance tfrac fractional part by tstep frac

	sbbl	%ecx,%ecx			// turn tstep carry into -1 (0 if none)
	addl	%ebp,%ebx			// advance sfrac fractional part by sstep frac
	adcl	advancetable+4(,%ecx,4),%esi	// point to next source texel

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	(%esi),%al
	addl	%ebp,%ebx
	movb	%al,1(%edi)
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,2(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,3(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi


//
// start FDIV for end of next segment in flight, so it can overlap
//
	movl	counttemp,%ecx
	cmpl	$8,%ecx			// more than one segment after this?
	ja		LSetupNotLast2	// yes

	decl	%ecx
	jz		LFDIVInFlight2	// if only one pixel, no need to start an FDIV
	movl	%ecx,spancountminus1
	fildl	spancountminus1

	flds	C(d_zistepu)		// C(d_zistepu) | spancountminus1
	fmul	%st(1),%st(0)	// C(d_zistepu)*scm1 | scm1
	flds	C(d_tdivzstepu)	// C(d_tdivzstepu) | C(d_zistepu)*scm1 | scm1
	fmul	%st(2),%st(0)	// C(d_tdivzstepu)*scm1 | C(d_zistepu)*scm1 | scm1
	fxch	%st(1)			// C(d_zistepu)*scm1 | C(d_tdivzstepu)*scm1 | scm1
	faddp	%st(0),%st(3)	// C(d_tdivzstepu)*scm1 | scm1
	fxch	%st(1)			// scm1 | C(d_tdivzstepu)*scm1
	fmuls	C(d_sdivzstepu)	// C(d_sdivzstepu)*scm1 | C(d_tdivzstepu)*scm1
	fxch	%st(1)			// C(d_tdivzstepu)*scm1 | C(d_sdivzstepu)*scm1
	faddp	%st(0),%st(3)	// C(d_sdivzstepu)*scm1
	flds	fp_64k			// 64k | C(d_sdivzstepu)*scm1
	fxch	%st(1)			// C(d_sdivzstepu)*scm1 | 64k
	faddp	%st(0),%st(4)	// 64k

	fdiv	%st(1),%st(0)	// this is what we've gone to all this trouble to
							//  overlap
	jmp		LFDIVInFlight2

	.align	4
LSetupNotLast2:
	fadds	zi8stepu
	fxch	%st(2)
	fadds	sdivz8stepu
	fxch	%st(2)
	flds	tdivz8stepu
	faddp	%st(0),%st(2)
	flds	fp_64k
	fdiv	%st(1),%st(0)	// z = 1/1/z
							// this is what we've gone to all this trouble to
							//  overlap
LFDIVInFlight2:
	movl	%ecx,counttemp

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,4(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,5(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,6(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi

	addl	$8,%edi
	movl	%edx,tfracf
	movl	snext,%edx
	movl	%ebx,sfracf
	movl	tnext,%ebx
	movl	%edx,s
	movl	%ebx,t

	movl	counttemp,%ecx		// retrieve count

//
// determine whether last span or not
//
	cmpl	$8,%ecx				// are there multiple segments remaining?
	movb	%al,-1(%edi)
	ja		LNotLastSegment		// yes

//
// last segment of scan
//
LLastSegment:

//
// advance s/z, t/z, and 1/z, and calculate s & t at end of span and steps to
// get there. The number of pixels left is variable, and we want to land on the
// last pixel, not step one past it, so we can't run into arithmetic problems
//
	testl	%ecx,%ecx
	jz		LNoSteps		// just draw the last pixel and we're done

// pick up after the FDIV that was left in flight previously


	fld		%st(0)			// duplicate it
	fmul	%st(4),%st(0)	// s = s/z * z
	fxch	%st(1)
	fmul	%st(3),%st(0)	// t = t/z * z
	fxch	%st(1)
	fistpl	snext
	fistpl	tnext

	movb	(%esi),%al		// load first texel in segment
	movl	C(tadjust),%ebx
	movb	%al,(%edi)		// store first pixel in segment
	movl	C(sadjust),%eax

	addl	snext,%eax
	addl	tnext,%ebx

	movl	C(bbextents),%ebp
	movl	C(bbextentt),%edx

	cmpl	$2048,%eax
	jl		LClampLow4
	cmpl	%ebp,%eax
	ja		LClampHigh4
LClampReentry4:
	movl	%eax,snext

	cmpl	$2048,%ebx
	jl		LClampLow5
	cmpl	%edx,%ebx
	ja		LClampHigh5
LClampReentry5:

	cmpl	$1,%ecx			// don't bother 
	je		LOnlyOneStep	// if two pixels in segment, there's only one step,
							//  of the segment length
	subl	s,%eax
	subl	t,%ebx

	addl	%eax,%eax		// convert to 15.17 format so multiply by 1.31
	addl	%ebx,%ebx		//  reciprocal yields 16.48

	imull	reciprocal_table-8(,%ecx,4) // sstep = (snext - s) / (spancount-1)
	movl	%edx,%ebp

	movl	%ebx,%eax
	imull	reciprocal_table-8(,%ecx,4) // tstep = (tnext - t) / (spancount-1)

LSetEntryvec:
//
// set up advancetable
//
	movl	entryvec_table(,%ecx,4),%ebx
	movl	%edx,%eax
	movl	%ebx,jumptemp		// entry point into code for RET later
	movl	%ebp,%ecx
	sarl	$16,%edx			// tstep >>= 16;
	movl	C(cachewidth),%ebx
	sarl	$16,%ecx			// sstep >>= 16;
	imull	%ebx,%edx

	addl	%ecx,%edx			// add in sstep
								// (tstep >> 16) * cachewidth + (sstep >> 16);
	movl	tfracf,%ecx
	movl	%edx,advancetable+4	// advance base in t
	addl	%ebx,%edx			// ((tstep >> 16) + 1) * cachewidth +
								//  (sstep >> 16);
	shll	$16,%ebp			// left-justify sstep fractional part
	movl	sfracf,%ebx
	shll	$16,%eax			// left-justify tstep fractional part
	movl	%edx,advancetable	// advance extra in t

	movl	%eax,tstep
	movl	%ecx,%edx
	addl	%eax,%edx
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi

	jmp		*jumptemp			// jump to the number-of-pixels handler

//----------------------------------------

LNoSteps:
	movb	(%esi),%al		// load first texel in segment
	subl	$7,%edi			// adjust for hardwired offset
	jmp		LEndSpan


LOnlyOneStep:
	subl	s,%eax
	subl	t,%ebx
	movl	%eax,%ebp
	movl	%ebx,%edx
	jmp		LSetEntryvec

//----------------------------------------

.globl	Entry2_8
Entry2_8:
	subl	$6,%edi		// adjust for hardwired offsets
	movb	(%esi),%al
	jmp		LLEntry2_8

//----------------------------------------

.globl	Entry3_8
Entry3_8:
	subl	$5,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	jmp		LLEntry3_8

//----------------------------------------

.globl	Entry4_8
Entry4_8:
	subl	$4,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LLEntry4_8

//----------------------------------------

.globl	Entry5_8
Entry5_8:
	subl	$3,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LLEntry5_8

//----------------------------------------

.globl	Entry6_8
Entry6_8:
	subl	$2,%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LLEntry6_8

//----------------------------------------

.globl	Entry7_8
Entry7_8:
	decl	%edi		// adjust for hardwired offsets
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
	jmp		LLEntry7_8

//----------------------------------------

.globl	Entry8_8
Entry8_8:
	addl	%eax,%edx
	movb	(%esi),%al
	sbbl	%ecx,%ecx
	addl	%ebp,%ebx
	adcl	advancetable+4(,%ecx,4),%esi

	addl	tstep,%edx
	sbbl	%ecx,%ecx
	movb	%al,1(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
LLEntry7_8:
	sbbl	%ecx,%ecx
	movb	%al,2(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
LLEntry6_8:
	sbbl	%ecx,%ecx
	movb	%al,3(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
LLEntry5_8:
	sbbl	%ecx,%ecx
	movb	%al,4(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
	addl	tstep,%edx
LLEntry4_8:
	sbbl	%ecx,%ecx
	movb	%al,5(%edi)
	addl	%ebp,%ebx
	movb	(%esi),%al
	adcl	advancetable+4(,%ecx,4),%esi
LLEntry3_8:
	movb	%al,6(%edi)
	movb	(%esi),%al
LLEntry2_8:

LEndSpan:

//
// clear s/z, t/z, 1/z from FP stack
//
	fstp %st(0)
	fstp %st(0)
	fstp %st(0)

	movl	pspantemp,%ebx				// restore spans pointer
	movl	espan_t_pnext(%ebx),%ebx	// point to next span
	testl	%ebx,%ebx			// any more spans?
	movb	%al,7(%edi)
	jnz		LSpanLoop			// more spans

	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	ret

//----------------------------------------------------------------------
// 8-bpp horizontal span z drawing codefor polygons, with no transparency.
//
// Assumes there is at least one span in pzspans, and that every span
// contains at least one pixel
//----------------------------------------------------------------------

	.text

// z-clamp on a non-negative gradient span
LClamp:
	movl	$0x40000000,%edx
	xorl	%ebx,%ebx
	fstp	%st(0)
	jmp		LZDraw

// z-clamp on a negative gradient span
LClampNeg:
	movl	$0x40000000,%edx
	xorl	%ebx,%ebx
	fstp	%st(0)
	jmp		LZDrawNeg


#define pzspans	4+16

.globl C(D_DrawZSpans)
C(D_DrawZSpans):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi
	pushl	%esi				// preserve register variables
	pushl	%ebx

	flds	C(d_zistepu)
	movl	C(d_zistepu),%eax
	movl	pzspans(%esp),%esi
	testl	%eax,%eax
	jz		LFNegSpan

	fmuls	Float2ToThe31nd
	fistpl	izistep		// note: we are relying on FP exceptions being turned
						// off here to avoid range problems
	movl	izistep,%ebx	// remains loaded for all spans

LFSpanLoop:
// set up the initial 1/z value
	fildl	espan_t_v(%esi)
	fildl	espan_t_u(%esi)
	movl	espan_t_v(%esi),%ecx
	movl	C(d_pzbuffer),%edi
	fmuls	C(d_zistepu)
	fxch	%st(1)
	fmuls	C(d_zistepv)
	fxch	%st(1)
	fadds	C(d_ziorigin)
	imull	C(d_zrowbytes),%ecx
	faddp	%st(0),%st(1)

// clamp if z is nearer than 2 (1/z > 0.5)
	fcoms	float_point5
	addl	%ecx,%edi
	movl	espan_t_u(%esi),%edx
	addl	%edx,%edx				// word count
	movl	espan_t_count(%esi),%ecx
	addl	%edx,%edi				// pdest = &pdestspan[scans->u];
	pushl	%esi		// preserve spans pointer
	fnstsw	%ax
	testb	$0x45,%ah
	jz		LClamp

	fmuls	Float2ToThe31nd
	fistpl	izi			// note: we are relying on FP exceptions being turned
						// off here to avoid problems when the span is closer
						// than 1/(2**31)
	movl	izi,%edx

// at this point:
// %ebx = izistep
// %ecx = count
// %edx = izi
// %edi = pdest

LZDraw:

// do a single pixel up front, if necessary to dword align the destination
	testl	$2,%edi
	jz		LFMiddle
	movl	%edx,%eax
	addl	%ebx,%edx
	shrl	$16,%eax
	decl	%ecx
	movw	%ax,(%edi)
	addl	$2,%edi

// do middle a pair of aligned dwords at a time
LFMiddle:
	pushl	%ecx
	shrl	$1,%ecx				// count / 2
	jz		LFLast				// no aligned dwords to do
	shrl	$1,%ecx				// (count / 2) / 2
	jnc		LFMiddleLoop		// even number of aligned dwords to do

	movl	%edx,%eax
	addl	%ebx,%edx
	shrl	$16,%eax
	movl	%edx,%esi
	addl	%ebx,%edx
	andl	$0xFFFF0000,%esi
	orl		%esi,%eax
	movl	%eax,(%edi)
	addl	$4,%edi
	andl	%ecx,%ecx
	jz		LFLast

LFMiddleLoop:
	movl	%edx,%eax
	addl	%ebx,%edx
	shrl	$16,%eax
	movl	%edx,%esi
	addl	%ebx,%edx
	andl	$0xFFFF0000,%esi
	orl		%esi,%eax
	movl	%edx,%ebp
	movl	%eax,(%edi)
	addl	%ebx,%edx
	shrl	$16,%ebp
	movl	%edx,%esi
	addl	%ebx,%edx
	andl	$0xFFFF0000,%esi
	orl		%esi,%ebp
	movl	%ebp,4(%edi)	// FIXME: eliminate register contention
	addl	$8,%edi

	decl	%ecx
	jnz		LFMiddleLoop

LFLast:
	popl	%ecx			// retrieve count
	popl	%esi			// retrieve span pointer

// do the last, unaligned pixel, if there is one
	andl	$1,%ecx			// is there an odd pixel left to do?
	jz		LFSpanDone		// no
	shrl	$16,%edx
	movw	%dx,(%edi)		// do the final pixel's z

LFSpanDone:
	movl	espan_t_pnext(%esi),%esi
	testl	%esi,%esi
	jnz		LFSpanLoop

	jmp		LFDone

LFNegSpan:
	fmuls	FloatMinus2ToThe31nd
	fistpl	izistep		// note: we are relying on FP exceptions being turned
						// off here to avoid range problems
	movl	izistep,%ebx	// remains loaded for all spans

LFNegSpanLoop:
// set up the initial 1/z value
	fildl	espan_t_v(%esi)
	fildl	espan_t_u(%esi)
	movl	espan_t_v(%esi),%ecx
	movl	C(d_pzbuffer),%edi
	fmuls	C(d_zistepu)
	fxch	%st(1)
	fmuls	C(d_zistepv)
	fxch	%st(1)
	fadds	C(d_ziorigin)
	imull	C(d_zrowbytes),%ecx
	faddp	%st(0),%st(1)

// clamp if z is nearer than 2 (1/z > 0.5)
	fcoms	float_point5
	addl	%ecx,%edi
	movl	espan_t_u(%esi),%edx
	addl	%edx,%edx				// word count
	movl	espan_t_count(%esi),%ecx
	addl	%edx,%edi				// pdest = &pdestspan[scans->u];
	pushl	%esi		// preserve spans pointer
	fnstsw	%ax
	testb	$0x45,%ah
	jz		LClampNeg

	fmuls	Float2ToThe31nd
	fistpl	izi			// note: we are relying on FP exceptions being turned
						// off here to avoid problems when the span is closer
						// than 1/(2**31)
	movl	izi,%edx

// at this point:
// %ebx = izistep
// %ecx = count
// %edx = izi
// %edi = pdest

LZDrawNeg:

// do a single pixel up front, if necessary to dword align the destination
	testl	$2,%edi
	jz		LFNegMiddle
	movl	%edx,%eax
	subl	%ebx,%edx
	shrl	$16,%eax
	decl	%ecx
	movw	%ax,(%edi)
	addl	$2,%edi

// do middle a pair of aligned dwords at a time
LFNegMiddle:
	pushl	%ecx
	shrl	$1,%ecx				// count / 2
	jz		LFNegLast			// no aligned dwords to do
	shrl	$1,%ecx				// (count / 2) / 2
	jnc		LFNegMiddleLoop		// even number of aligned dwords to do

	movl	%edx,%eax
	subl	%ebx,%edx
	shrl	$16,%eax
	movl	%edx,%esi
	subl	%ebx,%edx
	andl	$0xFFFF0000,%esi
	orl		%esi,%eax
	movl	%eax,(%edi)
	addl	$4,%edi
	andl	%ecx,%ecx
	jz		LFNegLast

LFNegMiddleLoop:
	movl	%edx,%eax
	subl	%ebx,%edx
	shrl	$16,%eax
	movl	%edx,%esi
	subl	%ebx,%edx
	andl	$0xFFFF0000,%esi
	orl		%esi,%eax
	movl	%edx,%ebp
	movl	%eax,(%edi)
	subl	%ebx,%edx
	shrl	$16,%ebp
	movl	%edx,%esi
	subl	%ebx,%edx
	andl	$0xFFFF0000,%esi
	orl		%esi,%ebp
	movl	%ebp,4(%edi)	// FIXME: eliminate register contention
	addl	$8,%edi

	decl	%ecx
	jnz		LFNegMiddleLoop

LFNegLast:
	popl	%ecx			// retrieve count
	popl	%esi			// retrieve span pointer

// do the last, unaligned pixel, if there is one
	andl	$1,%ecx			// is there an odd pixel left to do?
	jz		LFNegSpanDone	// no
	shrl	$16,%edx
	movw	%dx,(%edi)		// do the final pixel's z

LFNegSpanDone:
	movl	espan_t_pnext(%esi),%esi
	testl	%esi,%esi
	jnz		LFNegSpanLoop

LFDone:
	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	ret

#endif	// id386
