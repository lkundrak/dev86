!
! This floppy bootblock bypasses the floppy boot ...
!

BOOTDISK = 0x80
ORGADDR = $0600

use16
org ORGADDR
start:
include sysboot.s

org dos_sysid
   .ascii "BOOT C"       ! System ID

org codestart
  cli			! Assume _nothing_!
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
  sti			! Let the interrupts back in.

  mov	si,#mesg
  call	prtmsg

  mov	di,#5
hcode:
  mov	bx,#$7C00		! Pointer to start of BB.
  mov	ax,#$0201		! Read 1 sector
  mov	cx,#$0001		! From sector 1
  mov	dx,#BOOTDISK		! Of the hard drive head zero
  int	$13
  jc	error
  jmpi	$7c00,0

error:
  mov	si,#mesg2
  call	prtmsg

  dec	di
  jz	reboot

  mov	si,#mesg3
  call	prtmsg
  jmp	hcode

prtmsg:
  lodsb
  cmp	al,#0
  jz	EOS
  mov	bx,#7
  mov	ah,#$E		! Can't use $13 cause that's AT+ only!
  int	$10
  jmp	prtmsg
EOS:
  ret

reboot:
  mov	si,#mesg4
  call	prtmsg

  xor	ax,ax
  int	$16
  jmpi	$0,$FFFF	! Reboot.

mesg2:	.asciz	"Disk error\r\n"
mesg3:	.asciz	"Retrying\r\n"
mesg4:	.asciz	"Press a key:"

export mesg
 if BOOTDISK = 0x80
mesg:	.asciz	"Bypassing floppy boot\r\n"
 else
mesg:	.asciz	"Booting drive two\r\n"
 endif

