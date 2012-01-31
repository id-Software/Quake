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

