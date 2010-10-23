
! The master boot sector will have setup a stack,
! this is normally at 0:7c00 down.
! DS, SS, CS and ES will all have value 0 so the execution address is 0:7c00
! On entry the register SI will be pointer to the partition entry that
! this sector was loaded from, DL is the drive.

! Also if it's a standard Master boot DH will be the head, CX will be the
! sector and cylinder, BX=7C00, AX=1, DI=7DFE, BP=SI. There's a reasonable
! chance that this isn't true though.

! The Master boot itself will have been loaded and run at $07c00
! The BIOS must have setup a stack because interrupts are enabled
! Little else can be assumed because DOS doesn`t assume anything either

! Lowest available is $0500, MSDOS appears to use $0600 ... I wonder why?
ORGADDR=$0500

org ORGADDR

sysboot_start:
  cli                   ! Assume _nothing_!
  cld
  mov   bx,#$7C00       ! Pointer to start of BB.
  xor   ax,ax           ! Segs all to zero
  mov   ds,ax
  mov   es,ax
  mov   ss,ax
  mov   sp,bx           ! SP Just below BB
  mov   cx,#$100        ! Move 256 words
  mov   si,bx           ! From default BB
  mov   di,#ORGADDR     ! To the correct address.
  rep
   movsw
  jmpi  cont,#0         ! Set CS:IP correct.
cont:
  sti                   ! Let the interrupts back in.

! This is where the code will be overlaid, the default is a hang
public codestart
codestart:
  j	codestart

! Partition table
public bootblock_magic

.blkb sysboot_start+0x1B6-*
table_start:
.blkb 2			! Dirty bits
.blkb 4			! Volume Serial Number
.blkb 2                 ! Possible Magic number

.blkb sysboot_start+0x1BE-*
partition_1:
.blkb 8			! IN,SH,SS,ST,OS,EH,ES,ET
.blkw 2			! Linear position (0 based)
.blkw 2			! Linear length
.blkb sysboot_start+0x1CE-*
partition_2:
.blkb 8			! IN,SH,SS,ST,OS,EH,ES,ET
.blkw 2			! Linear position (0 based)
.blkw 2			! Linear length
.blkb sysboot_start+0x1DE-*
partition_3:
.blkb 8			! IN,SH,SS,ST,OS,EH,ES,ET
.blkw 2			! Linear position (0 based)
.blkw 2			! Linear length
.blkb sysboot_start+0x1EE-*
partition_4:
.blkb 8			! IN,SH,SS,ST,OS,EH,ES,ET
.blkw 2			! Linear position (0 based)
.blkw 2			! Linear length

.blkb sysboot_start+0x1FE-*
bootblock_magic:
.blkb 2
