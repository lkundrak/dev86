
include sysmbr.s

org codestart
  mov	si,#message
nextc:
  lodsb
  cmp	al,#0
  jz	eos
  mov	bx,#7
  mov	ah,#$E		! Can't use $13 cause that's AT+ only!
  int	$10
  jmp	nextc
eos:			! Wait for a key then reboot
  xor	ax,ax
  int	$16
  jmpi	$0,$FFFF	! Wam! Try or die!

export message
message:
  .asciz	"MBR loaded but not bootable.\r\n"

include sysmbrtail.s
