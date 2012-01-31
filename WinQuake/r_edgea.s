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
// r_edgea.s
// x86 assembly-language edge-processing code.
//

#include "asm_i386.h"
#include "quakeasm.h"
#include "asm_draw.h"

#if	id386

	.data
Ltemp:					.long	0
float_1_div_0100000h:	.long	0x35800000	// 1.0/(float)0x100000
float_point_999:		.single	0.999
float_1_point_001:		.single	1.001

	.text

//--------------------------------------------------------------------

#define edgestoadd	4+8		// note odd stack offsets because of interleaving
#define edgelist	8+12	// with pushes

.globl C(R_EdgeCodeStart)
C(R_EdgeCodeStart):

.globl C(R_InsertNewEdges)
C(R_InsertNewEdges):
	pushl	%edi
	pushl	%esi				// preserve register variables
	movl	edgestoadd(%esp),%edx
	pushl	%ebx
	movl	edgelist(%esp),%ecx

LDoNextEdge:
	movl	et_u(%edx),%eax
	movl	%edx,%edi

LContinueSearch:
	movl	et_u(%ecx),%ebx
	movl	et_next(%ecx),%esi
	cmpl	%ebx,%eax
	jle		LAddedge
	movl	et_u(%esi),%ebx
	movl	et_next(%esi),%ecx
	cmpl	%ebx,%eax
	jle		LAddedge2
	movl	et_u(%ecx),%ebx
	movl	et_next(%ecx),%esi
	cmpl	%ebx,%eax
	jle		LAddedge
	movl	et_u(%esi),%ebx
	movl	et_next(%esi),%ecx
	cmpl	%ebx,%eax
	jg		LContinueSearch

LAddedge2:
	movl	et_next(%edx),%edx
	movl	et_prev(%esi),%ebx
	movl	%esi,et_next(%edi)
	movl	%ebx,et_prev(%edi)
	movl	%edi,et_next(%ebx)
	movl	%edi,et_prev(%esi)
	movl	%esi,%ecx

	cmpl	$0,%edx
	jnz		LDoNextEdge
	jmp		LDone

	.align 4
LAddedge:
	movl	et_next(%edx),%edx
	movl	et_prev(%ecx),%ebx
	movl	%ecx,et_next(%edi)
	movl	%ebx,et_prev(%edi)
	movl	%edi,et_next(%ebx)
	movl	%edi,et_prev(%ecx)

	cmpl	$0,%edx
	jnz		LDoNextEdge

LDone:
	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi

	ret

//--------------------------------------------------------------------

#define predge	4+4

.globl C(R_RemoveEdges)
C(R_RemoveEdges):
	pushl	%ebx
	movl	predge(%esp),%eax

Lre_loop:
	movl	et_next(%eax),%ecx
	movl	et_nextremove(%eax),%ebx
	movl	et_prev(%eax),%edx
	testl	%ebx,%ebx
	movl	%edx,et_prev(%ecx)
	jz		Lre_done
	movl	%ecx,et_next(%edx)

	movl	et_next(%ebx),%ecx
	movl	et_prev(%ebx),%edx
	movl	et_nextremove(%ebx),%eax
	movl	%edx,et_prev(%ecx)
	testl	%eax,%eax
	movl	%ecx,et_next(%edx)
	jnz		Lre_loop

	popl	%ebx
	ret

Lre_done:
	movl	%ecx,et_next(%edx)
	popl	%ebx

	ret

//--------------------------------------------------------------------

#define pedgelist	4+4		// note odd stack offset because of interleaving
							// with pushes

.globl C(R_StepActiveU)
C(R_StepActiveU):
	pushl	%edi
	movl	pedgelist(%esp),%edx
	pushl	%esi				// preserve register variables
	pushl	%ebx

	movl	et_prev(%edx),%esi

LNewEdge:
	movl	et_u(%esi),%edi

LNextEdge:
	movl	et_u(%edx),%eax
	movl	et_u_step(%edx),%ebx
	addl	%ebx,%eax
	movl	et_next(%edx),%esi
	movl	%eax,et_u(%edx)
	cmpl	%edi,%eax
	jl		LPushBack

	movl	et_u(%esi),%edi
	movl	et_u_step(%esi),%ebx
	addl	%ebx,%edi
	movl	et_next(%esi),%edx
	movl	%edi,et_u(%esi)
	cmpl	%eax,%edi
	jl		LPushBack2

	movl	et_u(%edx),%eax
	movl	et_u_step(%edx),%ebx
	addl	%ebx,%eax
	movl	et_next(%edx),%esi
	movl	%eax,et_u(%edx)
	cmpl	%edi,%eax
	jl		LPushBack

	movl	et_u(%esi),%edi
	movl	et_u_step(%esi),%ebx
	addl	%ebx,%edi
	movl	et_next(%esi),%edx
	movl	%edi,et_u(%esi)
	cmpl	%eax,%edi
	jnl		LNextEdge

LPushBack2:
	movl	%edx,%ebx
	movl	%edi,%eax
	movl	%esi,%edx
	movl	%ebx,%esi

LPushBack:
// push it back to keep it sorted
	movl	et_prev(%edx),%ecx
	movl	et_next(%edx),%ebx

// done if the -1 in edge_aftertail triggered this
	cmpl	$(C(edge_aftertail)),%edx
	jz		LUDone

// pull the edge out of the edge list
	movl	et_prev(%ecx),%edi
	movl	%ecx,et_prev(%esi)
	movl	%ebx,et_next(%ecx)

// find out where the edge goes in the edge list
LPushBackLoop:
	movl	et_prev(%edi),%ecx
	movl	et_u(%edi),%ebx
	cmpl	%ebx,%eax
	jnl		LPushBackFound

	movl	et_prev(%ecx),%edi
	movl	et_u(%ecx),%ebx
	cmpl	%ebx,%eax
	jl		LPushBackLoop

	movl	%ecx,%edi

// put the edge back into the edge list
LPushBackFound:
	movl	et_next(%edi),%ebx
	movl	%edi,et_prev(%edx)
	movl	%ebx,et_next(%edx)
	movl	%edx,et_next(%edi)
	movl	%edx,et_prev(%ebx)

	movl	%esi,%edx
	movl	et_prev(%esi),%esi

	cmpl	$(C(edge_tail)),%edx
	jnz		LNewEdge

LUDone:
	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi

	ret

//--------------------------------------------------------------------

#define surf	4		// note this is loaded before any pushes

	.align 4
TrailingEdge:
	movl	st_spanstate(%esi),%eax	// check for edge inversion
	decl	%eax
	jnz		LInverted

	movl	%eax,st_spanstate(%esi)
	movl	st_insubmodel(%esi),%ecx
	movl	0x12345678,%edx		// surfaces[1].st_next
LPatch0:
	movl	C(r_bmodelactive),%eax
	subl	%ecx,%eax
	cmpl	%esi,%edx
	movl	%eax,C(r_bmodelactive)
	jnz		LNoEmit				// surface isn't on top, just remove

// emit a span (current top going away)
	movl	et_u(%ebx),%eax
	shrl	$20,%eax				// iu = integral pixel u
	movl	st_last_u(%esi),%edx
	movl	st_next(%esi),%ecx
	cmpl	%edx,%eax
	jle		LNoEmit2				// iu <= surf->last_u, so nothing to emit

	movl	%eax,st_last_u(%ecx)	// surf->next->last_u = iu;
	subl	%edx,%eax
	movl	%edx,espan_t_u(%ebp)		// span->u = surf->last_u;

	movl	%eax,espan_t_count(%ebp)	// span->count = iu - span->u;
	movl	C(current_iv),%eax
	movl	%eax,espan_t_v(%ebp)		// span->v = current_iv;
	movl	st_spans(%esi),%eax
	movl	%eax,espan_t_pnext(%ebp)	// span->pnext = surf->spans;
	movl	%ebp,st_spans(%esi)			// surf->spans = span;
	addl	$(espan_t_size),%ebp

	movl	st_next(%esi),%edx		// remove the surface from the surface
	movl	st_prev(%esi),%esi		// stack

	movl	%edx,st_next(%esi)
	movl	%esi,st_prev(%edx)
	ret

LNoEmit2:
	movl	%eax,st_last_u(%ecx)	// surf->next->last_u = iu;
	movl	st_next(%esi),%edx		// remove the surface from the surface
	movl	st_prev(%esi),%esi		// stack

	movl	%edx,st_next(%esi)
	movl	%esi,st_prev(%edx)
	ret

LNoEmit:
	movl	st_next(%esi),%edx		// remove the surface from the surface
	movl	st_prev(%esi),%esi		// stack

	movl	%edx,st_next(%esi)
	movl	%esi,st_prev(%edx)
	ret

LInverted:
	movl	%eax,st_spanstate(%esi)
	ret

//--------------------------------------------------------------------

// trailing edge only
Lgs_trailing:
	pushl	$Lgs_nextedge
	jmp		TrailingEdge


.globl C(R_GenerateSpans)
C(R_GenerateSpans):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi
	pushl	%esi				// preserve register variables
	pushl	%ebx

// clear active surfaces to just the background surface
	movl	C(surfaces),%eax
	movl	C(edge_head_u_shift20),%edx
	addl	$(st_size),%eax
// %ebp = span_p throughout
	movl	C(span_p),%ebp

	movl	$0,C(r_bmodelactive)

	movl	%eax,st_next(%eax)
	movl	%eax,st_prev(%eax)
	movl	%edx,st_last_u(%eax)
	movl	C(edge_head)+et_next,%ebx		// edge=edge_head.next

// generate spans
	cmpl	$(C(edge_tail)),%ebx		// done if empty list
	jz		Lgs_lastspan

Lgs_edgeloop:

	movl	et_surfs(%ebx),%edi
	movl	C(surfaces),%eax
	movl	%edi,%esi
	andl	$0xFFFF0000,%edi
	andl	$0xFFFF,%esi
	jz		Lgs_leading		// not a trailing edge

// it has a left surface, so a surface is going away for this span
	shll	$(SURF_T_SHIFT),%esi
	addl	%eax,%esi
	testl	%edi,%edi
	jz		Lgs_trailing

// both leading and trailing
	call	TrailingEdge
	movl	C(surfaces),%eax

// ---------------------------------------------------------------
// handle a leading edge
// ---------------------------------------------------------------

Lgs_leading:
	shrl	$16-SURF_T_SHIFT,%edi
	movl	C(surfaces),%eax
	addl	%eax,%edi
	movl	0x12345678,%esi		// surf2 = surfaces[1].next;
LPatch2:
	movl	st_spanstate(%edi),%edx
	movl	st_insubmodel(%edi),%eax
	testl	%eax,%eax
	jnz		Lbmodel_leading

// handle a leading non-bmodel edge

// don't start a span if this is an inverted span, with the end edge preceding
// the start edge (that is, we've already seen the end edge)
	testl	%edx,%edx
	jnz		Lxl_done


// if (surf->key < surf2->key)
//		goto newtop;
	incl	%edx
	movl	st_key(%edi),%eax
	movl	%edx,st_spanstate(%edi)
	movl	st_key(%esi),%ecx
	cmpl	%ecx,%eax
	jl		Lnewtop

// main sorting loop to search through surface stack until insertion point
// found. Always terminates because background surface is sentinel
// do
// {
// 		surf2 = surf2->next;
// } while (surf->key >= surf2->key);
Lsortloopnb:
	movl	st_next(%esi),%esi
	movl	st_key(%esi),%ecx
	cmpl	%ecx,%eax
	jge		Lsortloopnb

	jmp		LInsertAndExit


// handle a leading bmodel edge
	.align	4
Lbmodel_leading:

// don't start a span if this is an inverted span, with the end edge preceding
// the start edge (that is, we've already seen the end edge)
	testl	%edx,%edx
	jnz		Lxl_done

	movl	C(r_bmodelactive),%ecx
	incl	%edx
	incl	%ecx
	movl	%edx,st_spanstate(%edi)
	movl	%ecx,C(r_bmodelactive)

// if (surf->key < surf2->key)
//		goto newtop;
	movl	st_key(%edi),%eax
	movl	st_key(%esi),%ecx
	cmpl	%ecx,%eax
	jl		Lnewtop

// if ((surf->key == surf2->key) && surf->insubmodel)
// {
	jz		Lzcheck_for_newtop

// main sorting loop to search through surface stack until insertion point
// found. Always terminates because background surface is sentinel
// do
// {
// 		surf2 = surf2->next;
// } while (surf->key > surf2->key);
Lsortloop:
	movl	st_next(%esi),%esi
	movl	st_key(%esi),%ecx
	cmpl	%ecx,%eax
	jg		Lsortloop

	jne		LInsertAndExit

// Do 1/z sorting to see if we've arrived in the right position
	movl	et_u(%ebx),%eax
	subl	$0xFFFFF,%eax
	movl	%eax,Ltemp
	fildl	Ltemp

	fmuls	float_1_div_0100000h // fu = (float)(edge->u - 0xFFFFF) *
								//      (1.0 / 0x100000);

	fld		%st(0)				// fu | fu
	fmuls	st_d_zistepu(%edi)	// fu*surf->d_zistepu | fu
	flds	C(fv)					// fv | fu*surf->d_zistepu | fu
	fmuls	st_d_zistepv(%edi)	// fv*surf->d_zistepv | fu*surf->d_zistepu | fu
	fxch	%st(1)				// fu*surf->d_zistepu | fv*surf->d_zistepv | fu
	fadds	st_d_ziorigin(%edi)	// fu*surf->d_zistepu + surf->d_ziorigin |
								//  fv*surf->d_zistepv | fu

	flds	st_d_zistepu(%esi)	// surf2->d_zistepu |
								//  fu*surf->d_zistepu + surf->d_ziorigin |
								//  fv*surf->d_zistepv | fu
	fmul	%st(3),%st(0)		// fu*surf2->d_zistepu |
								//  fu*surf->d_zistepu + surf->d_ziorigin |
								//  fv*surf->d_zistepv | fu
	fxch	%st(1)				// fu*surf->d_zistepu + surf->d_ziorigin |
								//  fu*surf2->d_zistepu |
								//  fv*surf->d_zistepv | fu
	faddp	%st(0),%st(2)		// fu*surf2->d_zistepu | newzi | fu

	flds	C(fv)					// fv | fu*surf2->d_zistepu | newzi | fu
	fmuls	st_d_zistepv(%esi)	// fv*surf2->d_zistepv |
								//  fu*surf2->d_zistepu | newzi | fu
	fld		%st(2)				// newzi | fv*surf2->d_zistepv |
								//  fu*surf2->d_zistepu | newzi | fu
	fmuls	float_point_999		// newzibottom | fv*surf2->d_zistepv |
								//  fu*surf2->d_zistepu | newzi | fu

	fxch	%st(2)				// fu*surf2->d_zistepu | fv*surf2->d_zistepv |
								//  newzibottom | newzi | fu
	fadds	st_d_ziorigin(%esi)	// fu*surf2->d_zistepu + surf2->d_ziorigin |
								//  fv*surf2->d_zistepv | newzibottom | newzi |
								//  fu
	faddp	%st(0),%st(1)		// testzi | newzibottom | newzi | fu
	fxch	%st(1)				// newzibottom | testzi | newzi | fu

// if (newzibottom >= testzi)
//     goto Lgotposition;

	fcomp	%st(1)				// testzi | newzi | fu

	fxch	%st(1)				// newzi | testzi | fu
	fmuls	float_1_point_001	// newzitop | testzi | fu
	fxch	%st(1)				// testzi | newzitop | fu

	fnstsw	%ax
	testb	$0x01,%ah
	jz		Lgotposition_fpop3

// if (newzitop >= testzi)
// {

	fcomp	%st(1)				// newzitop | fu
	fnstsw	%ax
	testb	$0x45,%ah
	jz		Lsortloop_fpop2

// if (surf->d_zistepu >= surf2->d_zistepu)
//     goto newtop;

	flds	st_d_zistepu(%edi)	// surf->d_zistepu | newzitop| fu
	fcomps	st_d_zistepu(%esi)	// newzitop | fu
	fnstsw	%ax
	testb	$0x01,%ah
	jz		Lgotposition_fpop2

	fstp	%st(0)				// clear the FPstack
	fstp	%st(0)
	movl	st_key(%edi),%eax
	jmp		Lsortloop


Lgotposition_fpop3:
	fstp	%st(0)
Lgotposition_fpop2:
	fstp	%st(0)
	fstp	%st(0)
	jmp		LInsertAndExit


// emit a span (obscures current top)

Lnewtop_fpop3:
	fstp	%st(0)
Lnewtop_fpop2:
	fstp	%st(0)
	fstp	%st(0)
	movl	st_key(%edi),%eax		// reload the sorting key

Lnewtop:
	movl	et_u(%ebx),%eax
	movl	st_last_u(%esi),%edx
	shrl	$20,%eax				// iu = integral pixel u
	movl	%eax,st_last_u(%edi)	// surf->last_u = iu;
	cmpl	%edx,%eax
	jle		LInsertAndExit			// iu <= surf->last_u, so nothing to emit

	subl	%edx,%eax
	movl	%edx,espan_t_u(%ebp)		// span->u = surf->last_u;

	movl	%eax,espan_t_count(%ebp)	// span->count = iu - span->u;
	movl	C(current_iv),%eax
	movl	%eax,espan_t_v(%ebp)		// span->v = current_iv;
	movl	st_spans(%esi),%eax
	movl	%eax,espan_t_pnext(%ebp)	// span->pnext = surf->spans;
	movl	%ebp,st_spans(%esi)			// surf->spans = span;
	addl	$(espan_t_size),%ebp

LInsertAndExit:
// insert before surf2
	movl	%esi,st_next(%edi)		// surf->next = surf2;
	movl	st_prev(%esi),%eax
	movl	%eax,st_prev(%edi)		// surf->prev = surf2->prev;
	movl	%edi,st_prev(%esi)		// surf2->prev = surf;
	movl	%edi,st_next(%eax)		// surf2->prev->next = surf;

// ---------------------------------------------------------------
// leading edge done
// ---------------------------------------------------------------

// ---------------------------------------------------------------
// see if there are any more edges
// ---------------------------------------------------------------

Lgs_nextedge:
	movl	et_next(%ebx),%ebx
	cmpl	$(C(edge_tail)),%ebx
	jnz		Lgs_edgeloop

// clean up at the right edge
Lgs_lastspan:

// now that we've reached the right edge of the screen, we're done with any
// unfinished surfaces, so emit a span for whatever's on top
	movl	0x12345678,%esi		// surfaces[1].st_next
LPatch3:
	movl	C(edge_tail_u_shift20),%eax
	xorl	%ecx,%ecx
	movl	st_last_u(%esi),%edx
	subl	%edx,%eax
	jle		Lgs_resetspanstate

	movl	%edx,espan_t_u(%ebp)
	movl	%eax,espan_t_count(%ebp)
	movl	C(current_iv),%eax
	movl	%eax,espan_t_v(%ebp)
	movl	st_spans(%esi),%eax
	movl	%eax,espan_t_pnext(%ebp)
	movl	%ebp,st_spans(%esi)
	addl	$(espan_t_size),%ebp

// reset spanstate for all surfaces in the surface stack
Lgs_resetspanstate:
	movl	%ecx,st_spanstate(%esi)
	movl	st_next(%esi),%esi
	cmpl	$0x12345678,%esi		// &surfaces[1]
LPatch4:
	jnz		Lgs_resetspanstate

// store the final span_p
	movl	%ebp,C(span_p)

	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	ret


// ---------------------------------------------------------------
// 1/z sorting for bmodels in the same leaf
// ---------------------------------------------------------------
	.align	4
Lxl_done:
	incl	%edx
	movl	%edx,st_spanstate(%edi)

	jmp		Lgs_nextedge


	.align	4
Lzcheck_for_newtop:
	movl	et_u(%ebx),%eax
	subl	$0xFFFFF,%eax
	movl	%eax,Ltemp
	fildl	Ltemp

	fmuls	float_1_div_0100000h // fu = (float)(edge->u - 0xFFFFF) *
								//      (1.0 / 0x100000);

	fld		%st(0)				// fu | fu
	fmuls	st_d_zistepu(%edi)	// fu*surf->d_zistepu | fu
	flds	C(fv)				// fv | fu*surf->d_zistepu | fu
	fmuls	st_d_zistepv(%edi)	// fv*surf->d_zistepv | fu*surf->d_zistepu | fu
	fxch	%st(1)				// fu*surf->d_zistepu | fv*surf->d_zistepv | fu
	fadds	st_d_ziorigin(%edi)	// fu*surf->d_zistepu + surf->d_ziorigin |
								//  fv*surf->d_zistepv | fu

	flds	st_d_zistepu(%esi)	// surf2->d_zistepu |
								//  fu*surf->d_zistepu + surf->d_ziorigin |
								//  fv*surf->d_zistepv | fu
	fmul	%st(3),%st(0)		// fu*surf2->d_zistepu |
								//  fu*surf->d_zistepu + surf->d_ziorigin |
								//  fv*surf->d_zistepv | fu
	fxch	%st(1)				// fu*surf->d_zistepu + surf->d_ziorigin |
								//  fu*surf2->d_zistepu |
								//  fv*surf->d_zistepv | fu
	faddp	%st(0),%st(2)		// fu*surf2->d_zistepu | newzi | fu

	flds	C(fv)				// fv | fu*surf2->d_zistepu | newzi | fu
	fmuls	st_d_zistepv(%esi)	// fv*surf2->d_zistepv |
								//  fu*surf2->d_zistepu | newzi | fu
	fld		%st(2)				// newzi | fv*surf2->d_zistepv |
								//  fu*surf2->d_zistepu | newzi | fu
	fmuls	float_point_999		// newzibottom | fv*surf2->d_zistepv |
								//  fu*surf2->d_zistepu | newzi | fu

	fxch	%st(2)				// fu*surf2->d_zistepu | fv*surf2->d_zistepv |
								//  newzibottom | newzi | fu
	fadds	st_d_ziorigin(%esi)	// fu*surf2->d_zistepu + surf2->d_ziorigin |
								//  fv*surf2->d_zistepv | newzibottom | newzi |
								//  fu
	faddp	%st(0),%st(1)		// testzi | newzibottom | newzi | fu
	fxch	%st(1)				// newzibottom | testzi | newzi | fu

// if (newzibottom >= testzi)
//     goto newtop;

	fcomp	%st(1)				// testzi | newzi | fu

	fxch	%st(1)				// newzi | testzi | fu
	fmuls	float_1_point_001	// newzitop | testzi | fu
	fxch	%st(1)				// testzi | newzitop | fu

	fnstsw	%ax
	testb	$0x01,%ah
	jz		Lnewtop_fpop3

// if (newzitop >= testzi)
// {

	fcomp	%st(1)				// newzitop | fu
	fnstsw	%ax
	testb	$0x45,%ah
	jz		Lsortloop_fpop2

// if (surf->d_zistepu >= surf2->d_zistepu)
//     goto newtop;

	flds	st_d_zistepu(%edi)	// surf->d_zistepu | newzitop | fu
	fcomps	st_d_zistepu(%esi)	// newzitop | fu
	fnstsw	%ax
	testb	$0x01,%ah
	jz		Lnewtop_fpop2

Lsortloop_fpop2:
	fstp	%st(0)				// clear the FP stack
	fstp	%st(0)
	movl	st_key(%edi),%eax
	jmp		Lsortloop


.globl C(R_EdgeCodeEnd)
C(R_EdgeCodeEnd):


//----------------------------------------------------------------------
// Surface array address code patching routine
//----------------------------------------------------------------------

	.align 4
.globl C(R_SurfacePatch)
C(R_SurfacePatch):

	movl	C(surfaces),%eax
	addl	$(st_size),%eax
	movl	%eax,LPatch4-4

	addl	$(st_next),%eax
	movl	%eax,LPatch0-4
	movl	%eax,LPatch2-4
	movl	%eax,LPatch3-4

	ret

#endif	// id386

