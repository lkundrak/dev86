ORGADDR=0x0600	  ! $0500..0600 stack or move ORGADDR down.

.org ORGADDR
entry start
public start
start:
  cld
  xor	ax,ax
  mov	si,#$7C00
  mov	di,#ORGADDR

  mov	ss,ax
  mov	sp,si	! ax, di or si

  push	ax
  pop	ds
  push	ax
  pop	es

  mov	cx,#256
  rep
   movsw
  jmpi	go,#0
go:

