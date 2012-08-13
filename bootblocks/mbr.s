!
! This is a 'Master Boot Record' following the MSDOS 'standards'.
! This BB successfully boots MSDOS, Windows or Linux in CHS or Linear.
!
! Copyright GPL2, Robert de Bath, 1996-2008.
!
! NB: This needs as86 0.16.0 or later
!
! Lowest available is $0500, MSDOS appears to use $0600 ... I wonder why?
ORGADDR=$0600
BOOTADDR=0x7c00

pbr=0	        ! Make this a partition boot record for ldboot.
direct=1        ! Direct boot from MBR to any sector.

copyright=0	! Add in the copyright message; if room.
mbrkey=0	! Option to choose the boot record based on keystroke (107)
message=1	! Display boot message (6+message space)

markptab=1	! Put an end marker just below the partition table.
use512=0	! Put the end marker at byte 510..512

linear=1	! Use the LBA BIOS addresses.
useCHS=1	! Disable CHS if you need space.
linCHS=1        ! Calculate CHS from linear mbr values.	 (41 bytes)

preboot=0	! Include the pre-boot loader. (40 bytes)

if linCHS
	! Allow immediate shifts etc.
	use16 186
endif
	mbr_extras=ORGADDR+0x1B6
	partition_start=ORGADDR+0x1BE
	partition_size=0x10
	partition_end=ORGADDR+0x1FE

if pbr|direct
	table_start=ORGADDR+0x1A2     ! Space for special table.
else
	table_start=mbr_extras
endif

export pbr
export direct
export linear
export useCHS
export linCHS
export mbrkey
export end_of_code

if preboot
export preboot
endif

org ORGADDR
    if pbr
	! Skip a potential MSDOS BPB.
boot_start:
	j	code
	nop        ! DOS appears to _require_ this to identify an MSDOS disk!!

	.blkb	boot_start+3-*
	.ascii	"LINUX"  ! System ID
	.byte	0,0,0
	.blkb	boot_start+0x5A-*
code:
    endif

	cli			! Assume _nothing_! (needed for NT 4)
	cld
	mov	bx,#BOOTADDR	! Pointer to start of BB.
	xor	ax,ax		! Segs all to zero
	mov	ss,ax
	mov	sp,bx		! SP Just below BB
    if pbr
	push	[si+10]		! Save the inbound disk offset and drive
	push	[si+8]		! As if called pbr(drive, offset)
	push	dx
    endif
	mov	ds,ax
	mov	es,ax
	mov	cx,#$100	! Move 256 words
	mov	si,bx		! From default BB
	mov	di,#ORGADDR	! To the correct address.
	rep
	 movsw
	jmpi	cont,#0		! Set CS:IP correct.
cont:
	sti			! Let the interrupts back in.

    if pbr
	pop	[boot_drive]
	pop	[boot_part]
	pop	[boot_part+2]
    endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! Next check for a pre-boot message, load or keypress
    if message
	call disp_message
    endif
    if preboot
	call preboot_code
    endif
    if mbrkey
	call 	key_wait
    endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!
  if (linear|useCHS)

    if pbr=0
! Now check the partition table, must use SI as pointer cause that's what the
! partition boot blocks expect.

! Normal active partition check, (Order: 1,2,3,4)
	mov	si,#partition_start
check_active:
	cmp	byte [si],#$80		! Flag for activated partition
	jz	found_active
    if direct=0
try_next_part:				! Only if no direct.
    endif
	add	si,#partition_size
	cmp	si,#partition_end
	jnz	check_active
    endif

    if pbr|direct
	mov	si,#table_start
	cmp	byte [si],#0
	jnz	found_active
try_next_part:
    endif

	jmp	no_partition

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! Active partition found, boot it.
found_active:
	mov	di,#6		! Max retries, int doc says 3 ... double it
	movb	[$7DFE],#0	! Clear magic for dosemu
retry:

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! If the BIOS has LBA extensions use them.
! If we have no CHS don't check for LBA just use it.
    if linear
	mov	dx,[si]		! dh = Drive head, dl = $80 ie HD drive 0

      if useCHS
	mov	ah,#$41
	mov	bx,#$55AA
       if mbrkey
	test	dl,#$80
	jz	do_CHS
       endif
	int	$13
	jc	do_CHS          ! Unknown call
	cmp	bx,#$AA55
	jnz	do_CHS          ! Wrong call
	test  cl,#1
	jz 	do_CHS		! EDD basic functions OK.
      endif

	push	si		! Save SI on read.

      if pbr|direct
	mov   ax,[si+8]	! 32bit disk address
	mov   cx,[si+10]
	cmp   si,#table_start
	jnz   normal_part
	add   ax,[boot_offset]
	adc   cx,[boot_offset+2]
normal_part:
      endif

	xor	bx,bx
	push  bx		! 64bit disk address
	push  bx
      if pbr|direct
	push  cx		! 32bit disk address
	push  ax
      else
	push  [si+10]		! 32bit disk address
	push  [si+8]
      endif
	push  bx		! Load segment
	push  #BOOTADDR	! Load address
	push  #1		! Number of sectors
	push  #16		! Packet size
	mov   si,sp
	mov	ah,#$42
	int	$13
	lea   sp,[si+16]

	pop	si
	jc	retry_error
	j	sector_loaded

    endif !linear

    if useCHS
do_CHS:

      if linCHS
	call  calc_chs
      else
	mov	dx,[si]		! dh = Drive head, dl = $80 ie HD drive 0
	mov	cx,[si+2]	! cx = Sector & head encoded for int $13
      endif

	mov   bx,#BOOTADDR    ! Pointer to start of BB.
	mov	ax,#$0201	! Read 1 sector
	int   $13		! Disk read.
	jnc	sector_loaded
    endif

! Error, reset and retry
retry_error:
	xor	ax,ax
	int	$13		! Disk reset

	dec	di
	jnz	retry		! Try again

	mov	si,#disk_read_error
	jmp	no_boot		! Sorry it ain't gonna work.

sector_loaded:
	mov	di,#$7DFE	! End of sector loaded
	cmp	[di],#$AA55	! Check for magic
	jnz	try_next_part	! No? Try next partition.

	mov	bp,si		! LILO says some BBs use bp rather than si
	jmpi	#BOOTADDR,#0	! Go!

no_partition:
	mov	si,#no_bootpart
no_boot:		! SI now has pointer to error message
	call	puts

  endif !(linear|useCHS)

! Fatal errors ...........
  if mbrkey
	mov	si,#crlf
	call	puts
	j	key_pause

  else

	mov	si,#press_key
	call	puts
keyboot:		! Wait for a key then reboot
	xor	ax,ax
	int	$16
	jmpi	$0,$FFFF	! Reboot.
  endif

  if useCHS
    if linCHS

calc_chs:               ! From lilo. 
	mov   ah,#8
	int   $13	        ! Drive Geom
	shr   dx,#8
	xchg  ax,dx
	inc   ax              ! AX = No. Heads
	dec   cx		! Davide BIOS bug: CX=0 => Sectors=64
	and   cx,#$3f         ! CX = Sectors 
	inc   cx

	mul   cx
	xchg  ax,bx           ! BX = Cylinder size

	mov   ax,[si+8]       ! Linear partition address.
	mov   dx,[si+10]

      if pbr|direct
	cmp   si,#table_start
	jnz   std_part
	add   ax,[boot_offset]
	adc   dx,[boot_offset+2]
std_part:
      endif

	div   bx              ! AX = Cyl, DX = head & sect

	shl   ah,#6
	xchg  ah,al
	xchg  dx,ax
	div   cl              ! AH = sect-1, AL = Head
	or    dl,ah           ! merge for CX arg.
	mov   cx,dx   
	inc   cx              ! Adjust sector No.

	mov   dx,[si]         ! dh = Orig Drive head, dl = $80 ie HD drive 0

	mov   dh,al           ! Head No.

	; CX & DX ready for int $13
	ret

    endif
  endif
!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! 
  if message
disp_message:
	mov	si,#Banner
  endif

! Display message uses SI,AX,BX
puts:			! This is smaller than using $13
	lodsb
	cmp	al,#0
	jz	.EOS
	mov	bx,#7
	mov	ah,#$E
	int	$10
	jmp	puts
.EOS:
	ret

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!

  if mbrkey=0
press_key:
	.asciz	"\r\nPress return:"
  endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!

  if (linear|useCHS)
    if mbrkey
disk_read_error:
no_bootpart:
	.asciz	"Boot error"
    else
disk_read_error:
	.asciz	"Disk read error"
no_bootpart:
	.asciz	"No bootable partition"
    endif
  endif !(linear|useCHS)

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! Choose the partition based on a pressed key ...

  if mbrkey
key_wait:
	mov	si,#Prompt
	call	puts
	call	wait_key
	jnz	key_pause

	mov	si,#Unprompt		! Nothing has happened, return.
	call	puts
	ret

key_pause:
	mov	si,#Pause
	call	puts

key_tick:
	call	wait_key
	jz	key_tick
	j	Got_key

wait_key:
	mov	di,#19			! Wait for 18-19 ticks

next_loop:
	mov	ah,#1
	int	$16
	jnz   done_wait
	mov	ah,#0
	int	$1A			! Get current tick
	cmp	dx,si			! If changed DEC our counter.
	jz	next_loop
	mov	si,dx
	dec	di
	jnz	next_loop

done_wait:
	ret

Got_key:
	mov	ah,#0			! Clean the kbd buffer.
	int	$16
	cmp	al,#0x20
	jz	key_tick

	push	ax
	mov	Showkey,al
	mov	si,#Showkey
	call	puts
	pop	ax

	! ... Now we use our key ...
	! 0 		=> Floppy
	! 1 .. 4	=> Hard disk partition.

    if useCHS
	cmp	al,#'F
	jz	is_floppy
	cmp	al,#'f
	jz	is_floppy
    endif

	cmp	al,#'1
	jb	key_pause
	cmp	al,#'4
	ja	key_pause

	and   ax,#0x7
	dec	ax
	mov	cl,#4
	shl	ax,cl
	add	ax,#partition_start
	mov	si,ax

! Set active flag for disk interrupt.
	or	byte [si],#$80
	br	found_active

    if useCHS
is_floppy:
	mov	si,#floppy_part
	br	found_active
    endif

Prompt:
	.asciz	"\rMBR: "
Unprompt:
	.asciz	"\r    \r"
Pause:
    if useCHS
	.asciz	"\rMBR F1234> "
    else
	.asciz	"\rMBR 1234> "
    endif
Showkey:
	.ascii	" "
crlf:
	.asciz	"\r\n"
floppy_part:
	.word	0,1

  endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! This is the pre-boot loader it uses CHS but that's ok for track 0
!
  if preboot
public preboot_code
preboot_code:
	mov	si,#pre_boot_table
	lodsw
	mov	di,ax		! First word is execute address
more_boot:
	lodsw
	test	ah,ah
	jz	load_done
	mov	bx,ax		! word 1 address
	lodsw
	mov	cx,ax		! word 2 CX, cylinder/sector
	lodsw
	mov	dx,ax		! word 3 DX, drive, head
	lodsw			! word 4 AX, $02, sector count
	int	$13
	jnc	more_boot	! This doesn't retry, with a HD it shouldn't be bad.

	mov	si,#disk_read_error
	br	no_boot		! Sorry it ain't gonna work.
load_done:
	call	di
exec_done:
return:
	ret

export pre_boot_table
pre_boot_table:
	! Example: Do nothing.
	.word return,0

	! If the message is in use, preallocate some extents
    if message
	.blkb 64
    endif

	! Labels
	! .word <execute address>
	! Then repeat ..
	! .word <BX>, <CX>, <DX>, <AX>
	! Or.
	! .word <Load Address>
	! .byte <sector> + (<cyl> & $300)>>2), <cyl> & $FF
	! .byte <Drive>, <Head>, <cnt>, 2
	! Finally
	! .word 0

	! Example: Load rest of H0,C0 into mem at BOOTADDR
	! .word BOOTADDR
	! .word BOOTADDR,$0002,$8000,$0210
	! .word $0000
  endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!
end_of_code:

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!

  if message
export Banner
Banner:
    if (linear|useCHS)
      if *<ORGADDR+0x100
	org ORGADDR+0x100
      endif
	.blkb	80	! At least 80 bytes for the message.
    else
	.asciz "This disk is not bootable."
    endif
  endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! Now make sure this isn't too big! Don't overlap disk serial numbers.
	if *>table_start
	 fail! Partition table overlaps
	endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! And a copyright message if there's room.
  if copyright
    if *<ORGADDR+0x170
	org ORGADDR+0x170
	.asciz "ELKS MBR       "
	.asciz "Robert de Bath,"
	.asciz "Copyright GPL2 "
	.asciz "1996-2008."
    endif
  endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!
  if pbr|direct
      if message
	org table_start-1
	.byte 0xFF
      else
	org table_start
      endif
boot_drive:
	.word 0
	.blkb 6
boot_part:
	.long 0
	.long 0
boot_offset:
	.long 0x7FFFFFFF

   if pbr=0
export boot_offset
export boot_drive
export boot_part
   endif
  endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! Clear the sector to the bottom of the partition table.
  if markptab
    if *<mbr_extras
	org mbr_extras-1
	.byte 0
    endif
  endif

	org mbr_extras     ! Dirty bit, Serial number
	.blkb 2
serial_no:
	.blkb 4

  if use512
	org ORGADDR+0x1BC
	.word 0xAA55
	org ORGADDR+0x1FE
	.word 0xAA55
  endif

! Sanity check.

  if pbr|direct
    if useCHS
      if linCHS=0
	fail !Raw CHS doesn't work with pbr or direct.
      endif
    endif
  endif

!THE END
