!----------------------------------------------------------------------------
!
! This is a skeleton for creating an impure Linux-8086 executable without
! using the linker. The .text and .data areas are correctly positioned.
!
! This file needs to be compiled using the 3 pass mode (-O)
! eg: as86 -O li86.s -s li86.sym -b li86.bin
!
!----------------------------------------------------------------------------
.text
org -32
.word	0x0301			! Magic
.word	0x0410			! Btype
.long	0x20			! header length
.long	_etext			! a_text
.long	_edata-_etext		! a_data
.long	0			! a_bss
.long	0			! a_entry
.long	STACK_SIZE		! a_total
.long	0			! a_syms
.data
.blkb	_etext
.even
.text
!----------------------------------------------------------------------------

STACK_SIZE = 0x10000

.data
var:
.word	$1234

.text
	int	$20
	mov	ax,var
	mov	bx,_edata
	push	ax
	ret

!----------------------------------------------------------------------------
! This trailer must be at the end of the file.
.text
_etext:
.data
_edata:
END

