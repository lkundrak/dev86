ORGADDR=$0500

org ORGADDR
  cld
  mov	bx,#$7C00	! Pointer to start of BB.
  xor	ax,ax		! Segs all to zero
  mov	ds,ax
  mov	es,ax
  mov	ss,ax
  mov	sp,bx		! SP Just below BB
  mov	cx,#$100	! Move 256 words
  mov	si,bx		! From default BB
  mov	di,#ORGADDR	! To the correct address.
  rep
   movsw
  jmpi	cont,#0		! Set CS:IP correct.
cont:
