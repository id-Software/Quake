LEnter16_16:
	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movw	0x12345678(,%eax,2),%ax
LBPatch0:
	addl	%ebp,%edx
	movw	%ax,(%edi)
	movw	0x12345678(,%ecx,2),%cx
LBPatch1:
	movw	%cx,2(%edi)
	addl	$0x4,%edi

	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movw	0x12345678(,%eax,2),%ax
LBPatch2:
	addl	%ebp,%edx
	movw	%ax,(%edi)
	movw	0x12345678(,%ecx,2),%cx
LBPatch3:
	movw	%cx,2(%edi)
	addl	$0x4,%edi

	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movw	0x12345678(,%eax,2),%ax
LBPatch4:
	addl	%ebp,%edx
	movw	%ax,(%edi)
	movw	0x12345678(,%ecx,2),%cx
LBPatch5:
	movw	%cx,2(%edi)
	addl	$0x4,%edi

	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movw	0x12345678(,%eax,2),%ax
LBPatch6:
	addl	%ebp,%edx
	movw	%ax,(%edi)
	movw	0x12345678(,%ecx,2),%cx
LBPatch7:
	movw	%cx,2(%edi)
	addl	$0x4,%edi

LEnter8_16:
	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movw	0x12345678(,%eax,2),%ax
LBPatch8:
	addl	%ebp,%edx
	movw	%ax,(%edi)
	movw	0x12345678(,%ecx,2),%cx
LBPatch9:
	movw	%cx,2(%edi)
	addl	$0x4,%edi

	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movw	0x12345678(,%eax,2),%ax
LBPatch10:
	addl	%ebp,%edx
	movw	%ax,(%edi)
	movw	0x12345678(,%ecx,2),%cx
LBPatch11:
	movw	%cx,2(%edi)
	addl	$0x4,%edi

LEnter4_16:
	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movw	0x12345678(,%eax,2),%ax
LBPatch12:
	addl	%ebp,%edx
	movw	%ax,(%edi)
	movw	0x12345678(,%ecx,2),%cx
LBPatch13:
	movw	%cx,2(%edi)
	addl	$0x4,%edi

LEnter2_16:
	movb	(%esi),%al
	movb	(%esi,%ebx,),%cl
	movb	%dh,%ah
	addl	%ebp,%edx
	movb	%dh,%ch
	leal	(%esi,%ebx,2),%esi
	movw	0x12345678(,%eax,2),%ax
LBPatch14:
	addl	%ebp,%edx
	movw	%ax,(%edi)
	movw	0x12345678(,%ecx,2),%cx
LBPatch15:
	movw	%cx,2(%edi)
	addl	$0x4,%edi
