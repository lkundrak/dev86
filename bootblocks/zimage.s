!
! This is a short helper to copy a zImage from high memory to low memory
! then call it.
!

  org	$1000
Start:
  xor	ax,ax
  mov	ds,ax
  mov	es,ax
  mov	ss,ax
  mov	sp,#Start

  mov	di,#8
nextpg:
  mov	cx,#$8000
  mov	ah,#$87
  mov	si,#GDT
  int	$15
  jnc	no_err

  ! If there's an error reset.
  jmpi	$0000,$FFFF

no_err:
  inc	srcaddr
  inc	destaddr
  dec	di
  jnz	nextpg

  ! Call the Image (same as bzImage)
go:

  ! Kill the floppy motor, needed in case the kernel has no floppy driver.
  mov	dx,#0x3f2
  xor	al, al
  outb

  ! Setup required registers and go ...
  mov	ax,#$9000
  mov	bx,#$4000-12	! Fix this to use boot_mem_top
  mov	es,ax
  mov	fs,ax
  mov	gs,ax
  mov	ds,ax
  mov	ss,ax
  mov	sp,bx

  jmpi	0,$9020		! Note SETUPSEG NOT INITSEG

! The global descriptor table.

GDT:
.word	$0000,$0000
.word	$0000,$0000

.word	$0000,$0000
.word	$0000,$0000

.word	$FFFF,$0000
srcaddr:
.word	$9310,$0000

.word	$FFFF,$0000
destaddr:
.word	$9301,$0000

.word	$0000,$0000
.word	$0000,$0000

.word	$0000,$0000
.word	$0000,$0000

