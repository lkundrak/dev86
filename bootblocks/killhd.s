!
! This program destroys the MBR and the start of the first partition of
! the hard disk. IT DOES NOT HAVE AN 'ARE YOU SURE?'
!

org $7c00

include sysboot.s

org dos_sysid
   .ascii "DEATH"       ! System ID

   org codestart

   cld
   xor	ax,ax
   mov	es,ax
   mov	ds,ax
   mov	ss,ax
   mov	sp,ax

   mov	di,#$8000
   mov	cx,#$0800
   rep
    stosw		! Zap a space.

   mov	dx,#$0080
   mov	cx,#$0001
   mov	bx,#$8000
   mov	ax,#$0308
   int	$13		! Zap the MBR (and a disk manager?)

   mov	dx,#$0180
   mov	cx,#$0001
   mov	bx,#$8000
   mov	ax,#$0308
   int	$13		! Zap the first partition boot and super.

!----------------------------------------------------------------

prtmsg:		! SI = pointer to error message
  mov	si,#boot_message

nextc:
  lodsb
  cmp	al,#0
  jz	eos
  mov	bx,#7
  mov	ah,#$E		! Can't use $13 cause that's AT+ only!
  int	$10
  jmp	nextc

!----------------------------------------------------------------

eos:			! Wait for a key then reboot
reboot:
  xor	ax,ax
  int	$16
  jmpi	$0,$FFFF	! Wam! Try or die!

export boot_message
boot_message:
  .asciz	"PANIC! OS Destroyed!\r\n"

