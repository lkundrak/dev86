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
   mov	cx,#$0400
   rep
    stosw		! Zap a space.

   mov	dx,#$0080
   mov	cx,#$0001
   mov	bx,#$8000
   mov	ax,#$0301
   int	$13		! Zap the MBR

   mov	dx,#$0180
   mov	cx,#$0001
   mov	bx,#$8000
   mov	ax,#$0304
   int	$13		! Zap the first partition boot and super.

   xor	ax,ax		! Wait for key
   int	$16
   jmpi	$0,$FFFF        ! Reboot

