/************************************************************************/
/* This file contains the BCC compiler helper functions */
/* (C) Copyright Bruce Evans */

#ifdef __AS386_16__
#asm
.text
export	_ldiv
_ldiv:
  push	bp
  mov	bp,sp
  push	di
  push	si
  mov	ax,[bp+6]
  mov	bx,[bp+8]
  mov	cx,[bp+10]
  mov	di,[bp+12]
  call	ldivmod
  mov	si,[bp+4]
  mov	[si],cx
  mov	[si+2],di
  mov	[si+4],ax
  mov	[si+6],bx
  mov	ax,si
  pop	si
  pop	di
  pop	bp
  ret
.data
.bss
#endasm
#endif

