!

org 0
  call	chk		!This chunk allows this code to exist at _any_ click
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
  mov	es,ax

! Print 'mesg'
  mov	ah,#0x03	! read cursor pos
  xor	bh,bh
  int	0x10

  mov	cx,#(emesg-mesg)
  mov	bp,#mesg
  mov	bx,#$7		! page 0, attribute 7 (normal)
  mov	ax,#$1301	! write string, move cursor
  int	$10

nogood:
  j	nogood

mesg:
.ascii	"Hello world"
emesg:

! Floppies aren't supposed to need this, oh well.
org 510
 .word $AA55
