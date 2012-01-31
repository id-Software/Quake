//
// worlda.s
// x86 assembly-language server testing stuff
//

#include "asm_i386.h"
#include "quakeasm.h"
//include "d_ifacea.h"

#if id386

	.data

Ltemp:	.long	0

	.text

//----------------------------------------------------------------------
// hull-point test
//----------------------------------------------------------------------

#define hull	4+8				// because only partially pushed
#define	num		8+4				// because only partially pushed
#define p		12+12			// because only partially pushed

	.align 4
.globl C(SV_HullPointContents)
C(SV_HullPointContents):
	pushl	%edi				// preserve register variables
	movl	num(%esp),%eax
	testl	%eax,%eax
	js		Lhquickout

//	float		d;
//	dclipnode_t	*node;
//	mplane_t	*plane;

	pushl	%ebx
	movl	hull(%esp),%ebx

	pushl	%ebp
	movl	p(%esp),%edx

	movl	hu_clipnodes(%ebx),%edi
	movl	hu_planes(%ebx),%ebp

	subl	%ebx,%ebx
	pushl	%esi

// %ebx: 0
// %eax: num
// %edx: p
// %edi: hull->clipnodes
// %ebp: hull->planes

//	while (num >= 0)
//	{

Lhloop:

//		node = hull->clipnodes + num;
//		plane = hull->planes + node->planenum;
// !!! if the size of dclipnode_t changes, the scaling of %eax needs to be
//     changed !!!
	movl	nd_planenum(%edi,%eax,8),%ecx
	movl	nd_children(%edi,%eax,8),%eax
	movl	%eax,%esi
	rorl	$16,%eax
	leal	(%ecx,%ecx,4),%ecx

//		if (plane->type < 3)
//			d = p[plane->type] - plane->dist;
	movl	pl_type(%ebp,%ecx,4),%bl
	cmpb	$3,%bl
	jb		Lnodot

//		else
//			d = DotProduct (plane->normal, p) - plane->dist;
	flds	pl_normal(%ebp,%ecx,4)
	fmuls	0(%edx)
	flds	pl_normal+4(%ebp,%ecx,4)
	fmuls	4(%edx)
	flds	pl_normal+8(%ebp,%ecx,4)
	fmuls	8(%edx)
	fxch	%st(1)
	faddp	%st(0),%st(2)
	faddp	%st(0),%st(1)
	fsubs	pl_dist(%ebp,%ecx,4)
	jmp		Lsub

Lnodot:
	flds	pl_dist(%ebp,%ecx,4)
	fsubrs	(%edx,%ebx,4)

Lsub:
	sarl	$16,%eax
	sarl	$16,%esi

//		if (d < 0)
//			num = node->children[1];
//		else
//			num = node->children[0];
	fstps	Ltemp
	movl	Ltemp,%ecx
	sarl	$31,%ecx
	andl	%ecx,%esi
	xorl	$0xFFFFFFFF,%ecx
	andl	%ecx,%eax
	orl		%esi,%eax
	jns		Lhloop

//	return num;
Lhdone:
	popl	%esi
	popl	%ebp
	popl	%ebx				// restore register variables

Lhquickout:
	popl	%edi

	ret

#endif	// id386

