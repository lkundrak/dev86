/************************************************************************/
/* This file contains the BCC compiler helper functions */
/* (C) Prentice Hall (Minix) http://www.cs.vu.nl/~ast/minix.html */
/* Miscellaneous obsolete junk 
 * __inport.o __inportb.o __outport.o __outportb.o __peekb.o __peekw.o
 * __pokeb.o __pokew.o
 */

#ifdef __AS368_16__
#if !__FIRST_ARG_IN_AX__
#asm
	.text	! This is common to all.
	.even
#endasm

/************************************************************************/
/* Function inport */

#ifdef L___inport
#asm

! int inport( int port );
! reads a word from the i/o port  port  and returns it

	.globl	_inport
_inport:
	pop	bx
	pop	dx
	dec	sp
	dec	sp
	inw
	jmp	bx
#endasm
#endif

/************************************************************************/
/* Function inportb */

#ifdef L___inportb
#asm

! int inportb( int port );
! reads a byte from the i/o port  port  and returns it

	.globl	_inportb
_inportb:
	pop	bx
	pop	dx
	dec	sp
	dec	sp
	in
	sub	ah,ah
	jmp	bx
#endasm
#endif

/************************************************************************/
/* Function outport */

#ifdef L___outport
#asm

! void outport( int port, int value );
! writes the word  value  to  the i/o port  port

	.globl	_outport
_outport:
	pop	bx
	pop	dx
	pop	ax
	sub	sp,*4
	outw
	jmp	bx
#endasm
#endif

/************************************************************************/
/* Function outportb */

#ifdef L___outportb
#asm

! void oportb( int port, char value );
! writes the byte  value  to  the i/o port  port
! this would be outportb except for feeble linkers

	.globl	_oportb
_oportb:
	pop	bx
	pop	dx
	pop	ax
	sub	sp,*4
	out
	jmp	bx
#endasm
#endif

/************************************************************************/
/* Function peekb */

#ifdef L___peekb
#asm

! int peekb( unsigned segment, char *offset );
! returns the (unsigned) byte at the far pointer  segment:offset

	.define	_peekb
_peekb:
	mov	cx,ds
	pop	dx
	pop	ds
	pop	bx
	sub	sp,*4
	movb	al,(bx)
	subb	ah,ah
	mov	ds,cx
	jmp	dx
#endasm
#endif

/************************************************************************/
/* Function peekw */

#ifdef L___peekw
#asm

! int peekw( unsigned segment, int *offset );
! returns the word at the far pointer  segment:offset

	.define	_peekw
_peekw:
	mov	cx,ds
	pop	dx
	pop	ds
	pop	bx
	sub	sp,*4
	mov	ax,(bx)
	mov	ds,cx
	jmp	dx
#endasm
#endif

/************************************************************************/
/* Function pokeb */

#ifdef L___pokeb
#asm

! void pokeb( unsigned segment, char *offset, char value );
! writes the byte  value  at the far pointer  segment:offset

	.define	_pokeb
_pokeb:
	mov	cx,ds
	pop	dx
	pop	ds
	pop	bx
	pop	ax
	sub	sp,*6
	movb	(bx),al
	mov	ds,cx
	jmp	dx
#endasm
#endif

/************************************************************************/
/* Function pokew */

#ifdef L___pokew
#asm

! void pokew( unsigned segment, int *offset, int value );
! writes the word value  at the far pointer  segment:offset

	.define	_pokew
_pokew:
	mov	cx,ds
	pop	dx
	pop	ds
	pop	bx
	pop	ax
	sub	sp,*6
	mov	(bx),ax
	mov	ds,cx
	jmp	dx
#endasm
#endif

#endif /* !__FIRST_ARG_IN_AX__ */
#endif
