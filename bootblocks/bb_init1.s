ORGADDR=0x0600

.org ORGADDR
entry start
public start
start:
  xor	ax,ax
  mov	si,#$7C00
  mov	di,#ORGADDR

  mov	ss,ax
  mov	sp,di	! Or ax or si

  push	ax
  pop	ds
  push	ax
  pop	es

  mov	cx,#256
  cld
  rep
   movsw
  jmpi	go,#0
go:

