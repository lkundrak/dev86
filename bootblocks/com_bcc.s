
! This header assumes only that we're loaded at a 16 byte boundry

ENDOFF=4		! If you add code adjust this till it stops failing.

org 0
entry start
public start
start:
  call	chk		! This chunk allows this code to exist at _any_ click
chk:
  pop	ax
  mov	cl,#4
  shr	ax,cl
  mov	bx,cs
  add	ax,bx
  push	ax
  mov	bx,#going
  push	bx
  retf
going:
  mov	ds,ax

  add	ax,#ENDOFF+2	! New CS
  mov	bx,ax		! Saved
  mov	dx,[a_entry]	! Save the entry - zero
  mov	ax,[btype]
  and	ax,#$20		! Split I/D ?
  jz	impure
  mov	cl,#4
  mov	ax,[a_text]
  shr	ax,cl
impure:			! ax is now offset 'tween CS&DS
  add	ax,bx		! ax = DS
  mov	ss,ax
  mov	sp,[a_total]	! SS:SP is now ready for prog.
  mov	ds,ax
  xor	cx,cx		! argc, argv and envp = 0
  push	cx
  push	cx
  push	cx
  push	bx		! CS
  push	dx		! Entry address
  retf			! Gone.

! Check for overlap
end_of_code:
  if end_of_code>hitme
     fail! At end_of_code
  endif

.org ((ENDOFF)<<4)-1
hitme:
.byte 0xFF	! Marker

magic:		.space 2	! A.out header
btype:		.space 2
headerlen:	.space 4
a_text:		.space 4
a_data:		.space 4
a_bss:		.space 4
a_entry:	.space 4
a_total:	.space 4
a_syms:		.space 4
.org (ENDOFF+2)<<4		! Code start.
