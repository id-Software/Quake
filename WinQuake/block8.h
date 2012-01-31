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
LEnter16_8:
	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movb	0x12345678(%eax),%al
LBPatch0:
	addl	%ebp,%edx
	movb	%al,(%edi)
	movb	0x12345678(%ecx),%cl
LBPatch1:
	movb	%cl,1(%edi)
	addl	$0x2,%edi

	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movb	0x12345678(%eax),%al
LBPatch2:
	addl	%ebp,%edx
	movb	%al,(%edi)
	movb	0x12345678(%ecx),%cl
LBPatch3:
	movb	%cl,1(%edi)
	addl	$0x2,%edi

	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movb	0x12345678(%eax),%al
LBPatch4:
	addl	%ebp,%edx
	movb	%al,(%edi)
	movb	0x12345678(%ecx),%cl
LBPatch5:
	movb	%cl,1(%edi)
	addl	$0x2,%edi

	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movb	0x12345678(%eax),%al
LBPatch6:
	addl	%ebp,%edx
	movb	%al,(%edi)
	movb	0x12345678(%ecx),%cl
LBPatch7:
	movb	%cl,1(%edi)
	addl	$0x2,%edi

LEnter8_8:
	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movb	0x12345678(%eax),%al
LBPatch8:
	addl	%ebp,%edx
	movb	%al,(%edi)
	movb	0x12345678(%ecx),%cl
LBPatch9:
	movb	%cl,1(%edi)
	addl	$0x2,%edi

	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movb	0x12345678(%eax),%al
LBPatch10:
	addl	%ebp,%edx
	movb	%al,(%edi)
	movb	0x12345678(%ecx),%cl
LBPatch11:
	movb	%cl,1(%edi)
	addl	$0x2,%edi

LEnter4_8:
	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movb	0x12345678(%eax),%al
LBPatch12:
	addl	%ebp,%edx
	movb	%al,(%edi)
	movb	0x12345678(%ecx),%cl
LBPatch13:
	movb	%cl,1(%edi)
	addl	$0x2,%edi

LEnter2_8:
	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movb	0x12345678(%eax),%al
LBPatch14:
	addl	%ebp,%edx
	movb	%al,(%edi)
	movb	0x12345678(%ecx),%cl
LBPatch15:
	movb	%cl,1(%edi)
	addl	$0x2,%edi

