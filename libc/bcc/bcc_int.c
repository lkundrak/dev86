/************************************************************************/
/* This file contains the BCC compiler helper functions */
/* (C) Copyright Bruce Evans */
/* Support for integer arithmetic 
 * __idiv.o __idivu.o __imod.o __imodu.o __imul.o __isl.o __isr.o __isru.o
 */

#ifdef __AS386_16__
#asm
	.text	! This is common to all.
	.even
#endasm

/************************************************************************/
/* Function idiv */

#ifdef L___idiv
#asm

! idiv.s
! idiv_ doesn`t preserve dx (returns remainder in it)

	.globl idiv_

idiv_:
	cwd
	idiv	bx
	ret
#endasm
#endif

/************************************************************************/
/* Function idivu */

#ifdef L___idivu
#asm

! idivu.s
! idiv_u doesn`t preserve dx (returns remainder in it)

	.globl idiv_u

idiv_u:
	xor	dx,dx
	div	bx
	ret
#endasm
#endif

/************************************************************************/
/* Function imod */

#ifdef L___imod
#asm

! imod.s
! imod doesn`t preserve dx (returns quotient in it)

	.globl imod

imod:
	cwd
	idiv	bx
	mov	ax,dx	
	ret
#endasm
#endif

/************************************************************************/
/* Function imodu */

#ifdef L___imodu
#asm

! imodu.s
! imodu doesn`t preserve dx (returns quotient in it)

	.globl imodu

imodu:
	xor	dx,dx
	div	bx
	mov	ax,dx		! instruction queue full so xchg slower
	ret
#endasm
#endif

/************************************************************************/
/* Function imul */

#ifdef L___imul
#asm

! imul.s
! imul_, imul_u don`t preserve dx

	.globl imul_
	.globl imul_u

imul_:
imul_u:
	imul	bx
	ret
#endasm
#endif

/************************************************************************/
/* Function isl */

#ifdef L___isl
#asm

! isl.s
! isl, islu don`t preserve cl

	.globl isl
	.globl islu

isl:
islu:
	mov	cl,bl
	shl	ax,cl
	ret
#endasm
#endif

/************************************************************************/
/* Function isr */

#ifdef L___isr
#asm

! isr.s
! isr doesn`t preserve cl

	.globl isr

isr:
	mov	cl,bl
	sar	ax,cl
	ret
#endasm
#endif

/************************************************************************/
/* Function isru */

#ifdef L___isru
#asm

! isru.s
! isru doesn`t preserve cl

	.globl isru

isru:
	mov	cl,bl
	shr	ax,cl
	ret
#endasm
#endif

#endif
