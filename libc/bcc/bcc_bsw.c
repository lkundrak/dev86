/************************************************************************/
/* This file contains the BCC compiler helper functions */
/* (C) Copyright Bruce Evans */
/* Support for long arithmetic on big-endian (words-swapped) longs 
 * __laddb.o __landb.o __lcmpb.o __lcomb.o __ldecb.o __ldivb.o __ldivub.o
 * __leorb.o __lincb.o __lmodb.o __lmodub.o __lmulb.o __lnegb.o __lorb.o
 * __lslb.o __lsrb.o __lsrub.o __lsubb.o __ltstb.o
 */
#ifdef __AS386_16__
#asm
	.text	! This is common to all.
	.even
#endasm

/************************************************************************/
/* Function laddb */

#ifdef L___laddb
#asm

! laddb.s

	.globl	laddb
	.globl	laddub

laddb:
laddub:
	add	bx,2(di)
	adc	ax,(di)
	ret
#endasm
#endif

/************************************************************************/
/* Function landb */

#ifdef L___landb
#asm

! landb.s

	.globl	landb
	.globl	landub

landb:
landub:
	and	ax,(di)
	and	bx,2(di)
	ret
#endasm
#endif

/************************************************************************/
/* Function lcmpb */

#ifdef L___lcmpb
#asm

! lcmpb.s
! lcmpb, lcmpub don`t preserve ax

	.globl	lcmpb
	.globl	lcmpub

lcmpb:
lcmpub:
	sub	ax,(di)		! don`t need to preserve ax
	je	LCMP_NOT_SURE
	ret

	.even

LCMP_NOT_SURE:
	cmp	bx,2(di)
	jb	LCMP_B_AND_LT	! b (below) becomes lt (less than) as well
	jge	LCMP_EXIT	! ge and already ae
				! else make gt as well as a (above)
	inc	ax		! clear ov and mi, set ne for greater than
LCMP_EXIT:
	ret

	.even

LCMP_B_AND_LT:
	dec	ax		! clear ov, set mi and ne for less than
	ret
#endasm
#endif

/************************************************************************/
/* Function lcomb */

#ifdef L___lcomb
#asm

! lcomb.s

	.globl	lcomb
	.globl	lcomub

lcomb:
lcomub:
	not	ax
	not	bx
	ret
#endasm
#endif

/************************************************************************/
/* Function ldecb */

#ifdef L___ldecb
#asm

! ldecb.s

	.globl	ldecb
	.globl	ldecub

ldecb:
ldecub:
	cmp	2(bx),*0
	je	LDEC_BOTH
	dec	2(bx)
	ret

	.even

LDEC_BOTH:
	dec	2(bx)
	dec	(bx)
	ret
#endasm
#endif

/************************************************************************/
/* Function ldivb */

#ifdef L___ldivb
#asm

! ldivb.s
! ax:bx / (di):2(di), quotient ax:bx, remainder cx:di, dx not preserved

	.globl	ldivb
	.extern	ldivmod

ldivb:
	xchg	ax,bx
	mov	cx,2(di)
	mov	di,(di)
	call	ldivmod		! bx:ax / di:cx, quot di:cx, rem bx:ax
	xchg	ax,di
	xchg	bx,cx
	ret
#endasm
#endif

/************************************************************************/
/* Function ldivub */

#ifdef L___ldivub
#asm

! ldivub.s
! unsigned ax:bx / (di):2(di), quotient ax:bx,remainder cx:di, dx not preserved

	.globl	ldivub
	.extern	ludivmod

ldivub:
	xchg	ax,bx
	mov	cx,2(di)
	mov	di,(di)
	call	ludivmod	! unsigned bx:ax / di:cx, quot di:cx, rem bx:ax
	xchg	ax,di
	xchg	bx,cx
	ret
#endasm
#endif

/************************************************************************/
/* Function leorb */

#ifdef L___leorb
#asm

! leorb.s

	.globl	leorb
	.globl	leorub

leorb:
leorub:
	xor	ax,(di)
	xor	bx,2(di)
	ret
#endasm
#endif

/************************************************************************/
/* Function lincb */

#ifdef L___lincb
#asm

! lincb.s

	.globl	lincb
	.globl	lincub

lincb:
lincub:
	inc	2(bx)
	je	LINC_HIGH_WORD
	ret

	.even

LINC_HIGH_WORD:
	inc	(bx)
	ret
#endasm
#endif

/************************************************************************/
/* Function lmodb */

#ifdef L___lmodb
#asm

! lmodb.s
! ax:bx % (di):2(di), remainder ax:bx, quotient cx:di, dx not preserved

	.globl	lmodb
	.extern	ldivmod

lmodb:
	xchg	ax,bx
	mov	cx,2(di)
	mov	di,(di)
	call	ldivmod		! bx:ax / di:cx, quot di:cx, rem bx:ax
	xchg	ax,bx
	xchg	cx,di
	ret
#endasm
#endif

/************************************************************************/
/* Function lmodub */

#ifdef L___lmodub
#asm

! lmodub.s
! unsigned ax:bx / (di):2(di), remainder ax:bx,quotient cx:di, dx not preserved

	.globl	lmodub
	.extern	ludivmod

lmodub:
	xchg	ax,bx
	mov	cx,2(di)
	mov	di,(di)
	call	ludivmod	! unsigned bx:ax / di:cx, quot di:cx, rem bx:ax
	xchg	ax,bx
	xchg	cx,di
	ret
#endasm
#endif

/************************************************************************/
/* Function lmulb */

#ifdef L___lmulb
#asm

! lmulb.s
! lmulb, lmulub don`t preserve cx, dx

	.globl	lmulb
	.globl	lmulub

lmulb:
lmulub:
	mul	2(di)
	xchg	ax,bx
	mov	cx,ax
	mul	(di)
	add	bx,ax
	mov	ax,2(di)
	mul	cx
	add	bx,dx
	xchg	ax,bx
	ret
#endasm
#endif

/************************************************************************/
/* Function lnegb */

#ifdef L___lnegb
#asm

! lnegb.s

	.globl	lnegb
	.globl	lnegub

lnegb:
lnegub:
	neg	ax
	neg	bx
	sbb	ax,*0
	ret
#endasm
#endif

/************************************************************************/
/* Function lorb */

#ifdef L___lorb
#asm

! lorb.s

	.globl	lorb
	.globl	lorub

lorb:
lorub:
	or	ax,(di)
	or	bx,2(di)
	ret
#endasm
#endif

/************************************************************************/
/* Function lslb */

#ifdef L___lslb
#asm

! lslb.s
! lslb, lslub don`t preserve cx

	.globl	lslb
	.globl	lslub

lslb:
lslub:
	mov	cx,di
	jcxz	LSL_EXIT
	cmp	cx,*32
	jae	LSL_ZERO
LSL_LOOP:
	shl	bx,*1
	rcl	ax,*1
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
/* Function lsrb */

#ifdef L___lsrb
#asm

! lsrb.s
! lsrb doesn`t preserve cx

	.globl	lsrb

lsrb:
	mov	cx,di
	jcxz	LSR_EXIT
	cmp	cx,*32
	jae	LSR_SIGNBIT
LSR_LOOP:
	sar	ax,*1
	rcr	bx,*1
	loop	LSR_LOOP
LSR_EXIT:
	ret

	.even

LSR_SIGNBIT:
	mov	cx,*32		! equivalent to +infinity in this context
	j	LSR_LOOP
#endasm
#endif

/************************************************************************/
/* Function lsrub */

#ifdef L___lsrub
#asm

! lsrub.s
! lsrub doesn`t preserve cx

	.globl	lsrub

lsrub:
	mov	cx,di
	jcxz	LSRU_EXIT
	cmp	cx,*32
	jae	LSRU_ZERO
LSRU_LOOP:
	shr	ax,*1
	rcr	bx,*1
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
/* Function lsubb */

#ifdef L___lsubb
#asm

! lsubb.s

	.globl	lsubb
	.globl	lsubub

lsubb:
lsubub:
	sub	bx,2(di)
	sbb	ax,(di)
	ret
#endasm
#endif

/************************************************************************/
/* Function ltstb */

#ifdef L___ltstb
#asm

! ltstb.s
! ltstb, ltstub don`t preserve ax

	.globl	ltstb
	.globl	ltstub

ltstb:
ltstub:
	test	ax,ax
	je	LTST_NOT_SURE
	ret

	.even

LTST_NOT_SURE:
	test	bx,bx
	js	LTST_FIX_SIGN
	ret

	.even

LTST_FIX_SIGN:
	inc	ax		! clear ov and mi, set ne for greater than
	ret
#endasm
#endif

#endif

