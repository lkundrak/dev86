!
! This is a 'Master Boot Record' following the MSDOS 'standards'.
! This BB successfully boots MSDOS or Linux.
!
! In addition it can:
!   Display a message configure at install time.
!   Load and execute a small program before the boot blocks are checked.
!
! Or
!
! Space for 12 extra partitions in the 'Old Disk Manager' form that Linux 
! _does_ understand (unfortunatly there doesn't appear to be an fdisk that
! understands them.)
!
! NB: This needs as86 0.16.0 or later

! Lowest available is $0500, MSDOS appears to use $0600 ... I wonder why?
ORGADDR=$0500
copyright=1	! Add in the copyright message; if room.
preboot=0	! Include the pre-boot loader.
mbrkey=0	! Option to choose the boot record based on keystroke
message=1	! Display boot message
use512=0	! Put the end marker at byte 510..512
markptab=1	! Put an end marker just below the partition table.

diskman=0	! Disk manager partitions, allows 16 partitions but
		! don't overwrite this with a LILO BB.

linear=1	! Use the linear addresses not the CHS ones (if available)
useCHS=1	! Disable CHS if you need space.

partition_start=ORGADDR+0x1BE
partition_size=0x10
partition_end=ORGADDR+0x1FE

 if diskman
  ! Partition table start ...
  table_start=ORGADDR+0xFC
  low_partition=table_start+2
 else
  table_start=partition_start
 endif

export linear
export diskman
export useCHS
export mbrkey
export preboot

org ORGADDR
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

 if (linear|useCHS)

! Now check the partition table, must use SI as pointer cause that's what the
! partition boot blocks expect.

! If we're using diskman and we're short of space check the partitions in
! physical order. (Order. 4,3,2,1,5,6,7,8,9,10,11,12,13,14,15,16)

 if (diskman&linear&useCHS)

  mov	si,#partition_end
check_next:
  sub	si,#partition_size
  cmp	byte [si],#$80			! Flag for activated partition
  jz	found_active
  cmp	si,#low_partition
  jnz	check_next

 else

! Normal active partition check, (Order: 1,2,3,4)
  mov	si,#partition_start
check_active:
  cmp	byte [si],#$80			! Flag for activated partition
  jz	found_active
try_next_part:
  add	si,#partition_size
  cmp	si,#partition_end
  jnz	check_active

! Check for Disk manager partitions in the order that Linux numbers them.
 if diskman&~(linear&useCHS)
  cmp	word ptr diskman_magic,#$AA55
  jnz	no_diskman
  mov	si,#partition_start
check_next:
  sub	si,#partition_size
  cmp	byte [si],#$80			! Flag for activated partition
  jz	found_active
  cmp	si,#low_partition
  jnz	check_next

no_diskman:
 endif
 endif

 if mbrkey=0
bad_boot:
 endif
  mov	si,#no_bootpart		! Message & boot
  jmp	no_boot

! Active partition found, boot it.
found_active:
  mov	di,#6		! Max retries, int doc says 3 ... double it
  movb	[$7DFE],#0	! Clear magic for dosemu
retry:

! If the BIOS has LBA extensions use them.
 if linear
 if useCHS
  mov	ah,#$41
  mov	bx,#$55AA
  mov	dx,[si]		! dh = Drive head, dl = $80 ie HD drive 0
  push	si		! Save SI on read.
 if mbrkey
  test	dl,#$80
  jz	do_CHS
 endif
  int	$13
  jc	do_CHS
  cmp	bx,#$AA55
  jnz	do_CHS
 else
  mov	dx,[si]		! dh = Drive head, dl = $80 ie HD drive 0
  push	si		! Save SI
 endif
  mov	bx,#disk_address
  mov	ax,[si+8]
  mov	[bx],ax
  mov	ax,[si+10]
  mov	[bx+2],ax
  mov	si,#disk_packet
  mov	ah,#$42
  int	$13
  pop	si
  jc	retry_error
  j	sector_loaded
disk_packet:
  .byte	$10
  .byte	0
  .word	1
  .word	$7C00
  .word	0
disk_address:
  .long 0
  .long 0

 if useCHS
do_CHS:
  pop	si
 endif
 endif

if useCHS
  mov	dx,[si]		! dh = Drive head, dl = $80 ie HD drive 0
  mov	cx,[si+2]	! cx = Sector & head encoded for int $13
  ! bx is correct at $7C00

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
 if diskman
  jnz	bad_boot	! Can't try again, two places to return to.
 else
  jnz	try_next_part	! No? Try next partition.
 endif

  mov	bp,si		! LILO says some BBs use bp rather than si
  jmpi	#$7C00,#0	! Go!

 else
  mov	si,#no_bootpart		! Message & boot
 endif !(linear|useCHS)

! Fatal errors ...........
 if mbrkey
bad_boot:
  mov	si,#no_bootpart
no_boot:		! SI now has pointer to error message
  call	puts
  mov	si,#crlf
  call	puts
  j	key_pause

 else

no_boot:		! SI now has pointer to error message
  lodsb
  cmp	al,#0
  jz	EOS
  mov	bx,#7
  mov	ah,#$E		! Can't use $13 cause that's AT+ only!
  int	$10
  jmp	no_boot
EOS:
  cmp	si,#press_end	! After msg output 'press key' message
  jz	keyboot
  mov	si,#press_key
  jmp	no_boot

keyboot:		! Wait for a key then reboot
  xor	ax,ax
  int	$16
  jmpi	$0,$FFFF	! Reboot.

press_key:
  .asciz	"\r\nPress return:"
press_end:
 endif

no_bootpart:
  .asciz	"Bad partition"
disk_read_error:
  .asciz	"Read error"

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

 if message
disp_message:
  mov	si,#Banner
 endif

puts:
  lodsb
  cmp	al,#0
  jz	EOS
  push	bx
  mov	bx,#7
  mov	ah,#$E			! Can't use $13 cause that's AT+ only!
  int	$10
  pop	bx
  jmp	puts
EOS:
  ret

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
  push	bx
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
  pop	bx

return:
  ret

export pre_boot_table
pre_boot_table:
  ! Example: Do nothing.
  .word return,0

  ! Labels
  ! .word <execute address>
  ! Then repeat ..
  ! .word <BX>, <CX>, <DX>, <AX>
  ! Or.
  ! .word <Load Address>
  ! .byte <sector> + (<cyl> & $300)>>2), <cyl> & $FF, <Drive>, <Head>, <cnt>, 2
  ! Finally
  ! .word 0

  ! Example: Load rest of H0,C0 into mem at $7C00 (8k).
  ! .word $7C00
  ! .word $7C00,$0002,$8000,$0210
  ! .word $0000
 endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!
 if message&~mbrkey
disp_message:
  mov	si,#Banner

puts:
  lodsb
  cmp	al,#0
  jz	.EOS
  push	bx
  mov	bx,#7
  mov	ah,#$E			! Can't use $13 cause that's AT+ only!
  int	$10
  pop	bx
  jmp	puts
.EOS:
  ret
 endif

 if message
export Banner
Banner:
  .blkb	16	! 16 bytes for the message at least.
 endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! Now make sure this isn't too big!
end_of_code:
  if *>table_start
   fail! Partition table overlaps
  endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! The diskman magic number and empty DM partitions.
 if diskman
  org table_start
public diskman_magic
diskman_magic:
  .word 0xAA55
  .blkb 12*partition_size-1
 endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! And a copyright message if there's room.
 if copyright
 if *<ORGADDR+0x180
  org ORGADDR+0x180
.asciz "ELKS MBR       "
.asciz "Robert de Bath,"
.asciz "Copyright      "
.asciz "1996-2003.  "
  org partition_start-1
  .byte 0xFF
 endif
 endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! Clear the sector to the bottom of the partition table.
 if markptab
  if *<partition_start-1
  org partition_start-1
  .byte 0xFF
 endif
 endif

 if use512
  org ORGADDR+0x1FE
  .word 0xAA55
 endif

 if 1&~(useCHS|linear|preboot)
  fail! Errm, you can't boot anything without 'linear' or 'useCHS'
 endif

!THE END
