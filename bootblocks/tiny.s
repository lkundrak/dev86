!
! This floppy bootblock bypasses the floppy boot ...
!

BOOTSEG = 0x07c0
BOOTDISK = 0x80

! Apparently on startup the only things we can assume are that we start at
! `start` (ABS addr $07C00) and we`ll have a few bytes of stack storage...

! So first set DS=ES= 0x07c0
  include sysboot.s
  .blkb codestart-*

start:
  mov	ax,#BOOTSEG
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
  xor	si,si
  mov	di,#$0200
  mov	cx,#$0100
  rep
   movw
  jmpi	hcode,#BOOTSEG+$20

hcode:
  mov	ax,#$0201		! Read 1 sector
  xor	bx,bx			! In the boot area
  mov	cx,#$0001		! From sector 1
  mov	dx,#BOOTDISK		! Of the hard drive head zero
  int	$13
  jc	hcode			! Keep trying forever!
  jmpi	$7c00,0


mesg:
 if BOOTDISK = 0x80
.ascii	"Bypassing floppy boot\r\n"
 else
.ascii	"Booting drive two\r\n"
 endif
emesg:
