/************************************************************************/
/* This file contains the BCC compiler helper functions */
/* (C) Copyright Bruce Evans */
/* Support for long arithmetic on little-endian (normal) longs 
 * __laddl.o __landl.o __lcmpl.o __lcoml.o __ldecl.o __ldivl.o __ldivul.o
 * __leorl.o __lincl.o __lmodl.o __lmodul.o __lmull.o __lnegl.o __lorl.o
 * __lsll.o __lsrl.o __lsrul.o __lsubl.o __ltstl.o
 */

#ifdef __AS386_16__
#asm
	.text	! This is common to all.
	.even
#endasm

/************************************************************************/
/* Function laddl */

#ifdef L___laddl
#asm

! laddl.s

	.globl	laddl
	.globl	laddul

laddl:
laddul:
	add	ax,[di]
	adc	bx,2[di]
	ret
#endasm
#endif

/************************************************************************/
/* Function landl */

#ifdef L___landl
#asm

! landl.s

	.globl	landl
	.globl	landul

landl:
landul:
	and	ax,[di]
	and	bx,2[di]
	ret
#endasm
#endif

/************************************************************************/
/* Function lcmpl */

#ifdef L___lcmpl
#asm

! lcmpl.s
! lcmpl, lcmpul don`t preserve bx

	.globl	lcmpl
	.globl	lcmpul

lcmpl:
lcmpul:
	sub	bx,2[di]	
	je	LCMP_NOT_SURE
	ret

	.even

LCMP_NOT_SURE:
	cmp	ax,[di]
	jb	LCMP_B_AND_LT
	jge	LCMP_EXIT
			
	inc	bx
LCMP_EXIT:
	ret

	.even

LCMP_B_AND_LT:
	dec	bx
	ret
#endasm
#endif

/************************************************************************/
/* Function lcoml */

#ifdef L___lcoml
#asm

! lcoml.s

	.globl	lcoml
	.globl	lcomul

lcoml:
lcomul:
	not	ax
	not	bx
	ret
#endasm
#endif

/************************************************************************/
/* Function ldecl */

#ifdef L___ldecl
#asm

! ldecl.s

	.globl	ldecl
	.globl	ldecul

ldecl:
ldecul:
	cmp	word ptr [bx],*0
	je	LDEC_BOTH
	dec	word ptr [bx]
	ret

	.even

LDEC_BOTH:
	dec	word ptr [bx]
	dec	word ptr 2[bx]
	ret
#endasm
#endif

/************************************************************************/
/* Function ldivl */

#ifdef L___ldivl
#asm

! ldivl.s
! bx:ax / 2(di):(di), quotient bx:ax, remainder di:cx, dx not preserved

	.globl	ldivl
	.extern	ldivmod

ldivl:
	mov	cx,[di]
	mov	di,2[di]
	call	ldivmod		
	xchg	ax,cx
	xchg	bx,di
	ret

#endasm
#endif

/************************************************************************/
/* Function ldivul */

#ifdef L___ldivul
#asm

! ldivul.s
! unsigned bx:ax / 2(di):(di), quotient bx:ax,remainder di:cx, dx not preserved

	.globl	ldivul
	.extern	ludivmod

ldivul:
	mov	cx,[di]
	mov	di,2[di]
	call	ludivmod	
	xchg	ax,cx
	xchg	bx,di
	ret
#endasm
#endif

/************************************************************************/
/* Function leorl */

#ifdef L___leorl
#asm

! leorl.s

	.globl	leorl
	.globl	leorul

leorl:
leorul:
	xor	ax,[di]
	xor	bx,2[di]
	ret
#endasm
#endif

/************************************************************************/
/* Function lincl */

#ifdef L___lincl
#asm

! lincl.s

	.globl	lincl
	.globl	lincul

lincl:
lincul:
	inc	word ptr [bx]
	je	LINC_HIGH_WORD
	ret

	.even

LINC_HIGH_WORD:
	inc	word ptr 2[bx]
	ret
#endasm
#endif

/************************************************************************/
/* Function lmodl */

#ifdef L___lmodl
#asm

! lmodl.s
! bx:ax % 2(di):(di), remainder bx:ax, quotient di:cx, dx not preserved

	.globl	lmodl
	.extern	ldivmod

lmodl:
	mov	cx,[di]
	mov	di,2[di]
	call	ldivmod
	ret	
#endasm
#endif

/************************************************************************/
/* Function lmodul */

#ifdef L___lmodul
#asm

! lmodul.s
! unsigned bx:ax / 2(di):(di), remainder bx:ax,quotient di:cx, dx not preserved

	.globl	lmodul
	.extern	ludivmod

lmodul:
	mov	cx,[di]
	mov	di,2[di]
	call	ludivmod
	ret
#endasm
#endif

/************************************************************************/
/* Function lmull */

#ifdef L___lmull
#asm

! lmull.s
! lmull, lmulul don`t preserve cx, dx

	.globl	lmull
	.globl	lmulul

lmull:
lmulul:
	mov	cx,ax
	mul	word ptr 2[di]
	xchg	ax,bx
	mul	word ptr [di]
	add	bx,ax
	mov	ax,ptr [di]
	mul	cx
	add	bx,dx
	ret
#endasm
#endif

/************************************************************************/
/* Function lnegl */

#ifdef L___lnegl
#asm

! lnegl.s

	.globl	lnegl
	.globl	lnegul

lnegl:
lnegul:
	neg	bx
	neg	ax
	sbb	bx,*0
	ret
#endasm
#endif

/************************************************************************/
/* Function lorl */

#ifdef L___lorl
#asm

! lorl.s

	.globl	lorl
	.globl	lorul

lorl:
lorul:
	or	ax,[di]
	or	bx,2[di]
	ret
#endasm
#endif

/************************************************************************/
/* Function lsll */

#ifdef L___lsll
#asm

! lsll.s
! lsll, lslul don`t preserve cx

	.globl	lsll
	.globl	lslul

lsll:
lslul:
	mov	cx,di
	jcxz	LSL_EXIT
	cmp	cx,*32
	jae	LSL_ZERO
LSL_LOOP:
	shl	ax,*1
	rcl	bx,*1
	loop	LSL_LOOP
LSL_EXIT:
	ret

	.even

LSL_ZERO:
	xor	ax,ax
	mov	bx,ax
	ret
#endasm
#endif

/************************************************************************/
/* Function lsrl */

#ifdef L___lsrl
#asm

! lsrl.s
! lsrl doesn`t preserve cx

	.globl	lsrl

lsrl:
	mov	cx,di
	jcxz	LSR_EXIT
	cmp	cx,*32
	jae	LSR_SIGNBIT
LSR_LOOP:
	sar	bx,*1
	rcr	ax,*1
	loop	LSR_LOOP
LSR_EXIT:
	ret

	.even

LSR_SIGNBIT:
	mov	cx,*32	
	j	LSR_LOOP
#endasm
#endif

/************************************************************************/
/* Function lsrul */

#ifdef L___lsrul
#asm

! lsrul.s
! lsrul doesn`t preserve cx

	.globl	lsrul

lsrul:
	mov	cx,di
	jcxz	LSRU_EXIT
	cmp	cx,*32
	jae	LSRU_ZERO
LSRU_LOOP:
	shr	bx,*1
	rcr	ax,*1
	loop	LSRU_LOOP
LSRU_EXIT:
	ret

	.even

LSRU_ZERO:
	xor	ax,ax
	mov	bx,ax
	ret
#endasm
#endif

/************************************************************************/
/* Function lsubl */

#ifdef L___lsubl
#asm

! lsubl.s

	.globl	lsubl
	.globl	lsubul

lsubl:
lsubul:
	sub	ax,[di]
	sbb	bx,2[di]
	ret
#endasm
#endif

/************************************************************************/
/* Function ltstl */

#ifdef L___ltstl
#asm

! ltstl.s
! ltstl, ltstul don`t preserve bx

	.globl	ltstl
	.globl	ltstul

ltstl:
ltstul:
	test	bx,bx
	je	LTST_NOT_SURE
	ret

	.even

LTST_NOT_SURE:
	test	ax,ax
	js	LTST_FIX_SIGN
	ret

	.even

LTST_FIX_SIGN:
	inc	bx
	ret
#endasm
#endif

#endif
