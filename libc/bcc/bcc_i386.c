/************************************************************************/
/* This file contains the BCC compiler helper functions */
/* (C) Copyright Bruce Evans */
/* Support for 386 integer arithmetic 
 * __divsi3.o __idiv.o __idivu.o __imod.o __imodu.o __imul.o
 * __isl.o __isr.o __isru.o
 */

#ifdef __AS386_32__
#asm
	.text	! This is common to all.
	.align	4
#endasm

#ifdef L___divsi3
#asm
! divsi3.s
	.globl ___divsi3
 ___divsi3:
	push	edx
	mov	eax,[esp+4+4]
	cdq
	idiv	[esp+4+4+4]
	pop	edx
	ret

	.globl ___udivsi3
	.text
	.align	4

 ___udivsi3:
	push	edx
	mov	eax,[esp+4+4]
	sub	edx,edx
	div	[esp+4+4+4]
	pop	edx
	ret
#endasm
#endif

#ifdef L___idiv
#asm
! idiv.s
! idiv_ doesn`t preserve edx (returns remainder in it)

	.globl idiv_
idiv_:
	cdq
	idiv	ebx
	ret
#endasm
#endif

#ifdef L___idivu
#asm
! idivu.s
! idiv_u doesn`t preserve edx (returns remainder in it)

	.globl idiv_u
idiv_u:
	xor	edx,edx
	div	ebx
	ret
#endasm
#endif

#ifdef L___imod
#asm
! imod.s
! imod doesn`t preserve edx (returns quotient in it)

	.globl imod
imod:
	cdq
	idiv	ebx
	mov	eax,edx		! instruction queue full so xchg slower
	ret
#endasm
#endif

#ifdef L___imodu
#asm
! imodu.s
! imodu doesn`t preserve edx (returns quotient in it)

	.globl imodu
imodu:
	xor	edx,edx
	div	ebx
	mov	eax,edx		! instruction queue full so xchg slower
	ret
#endasm
#endif

#ifdef L___imul
#asm
! imul.s
! imul_, imul_u don`t preserve edx

	.globl imul_
	.globl imul_u
imul_:
imul_u:
	imul	ebx
	ret
#endasm
#endif

#ifdef L___isl
#asm
! isl.s
! isl, islu don`t preserve cl

	.globl isl
	.globl islu
isl:
islu:
	mov	cl,bl
	shl	eax,cl
	ret
#endasm
#endif

#ifdef L___isr
#asm
! isr.s
! isr doesn`t preserve cl

	.globl isr
isr:
	mov	cl,bl
	sar	eax,cl
	ret
#endasm
#endif

#ifdef L___isru
#asm
! isru.s
! isru doesn`t preserve cl

	.globl isru
isru:
	mov	cl,bl
	shr	eax,cl
	ret
#endasm
#endif

#endif
