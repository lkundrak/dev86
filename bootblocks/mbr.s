!
! This is a 'Master Boot Record' following the MSDOS 'standards'.
! This BB successfully boots MSDOS or Linux.
!
! In addition it has the facility to load and execute a small program
! before the boot blocks are checked.
!
! Or
!
! Space for 12 extra partitions in the 'DiskManager' form that Linux 
! _does_ understand.
!
! NB: This needs Dev86 0.15.2 or later

! Lowest available is $0500, MSDOS appears to use $0600 ... I wonder why?
ORGADDR=$0500
preboot=0	! Include the pre-boot loader.
mbrkey=0	! Option to choose the boot record base on keystroke
message=1	! Display boot message
diskman=0	! Disk manager partitions, allows 16 partitions but
		! don't overwrite this with a LILO BB.

linear=0	! Use the linear addresses not the CHS ones

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

! Next check for a pre-boot load or a keypress
 if message
  call disp_message
 endif
 if preboot
  call preboot_code
 endif
 if mbrkey
  call 	key_wait
 endif

! Now check the partition table, must use SI as pointer cause that's what the
! partition boot blocks expect.

  mov	si,#partition_start
check_active:
  cmp	byte [si],#$80			! Flag for activated partition
  jz	found_active
 if mbrkey=0
bad_boot:
 endif
  add	si,#partition_size
  cmp	si,#partition_end
  jnz	check_active

  ! Check for Disk manager partitions in the order that Linux numbers them.
 if diskman
  cmp	word ptr diskman_magic,#$55AA
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

  mov	si,#no_bootpart		! Message & boot
  jmp	no_boot

found_active:
 if linear
  call linearise
 else
  mov	di,#6		! Max retries, int list says 3 ... double it
  mov	dx,[si]		! dh = Drive head, dl = $80 ie HD drive 0
  mov	cx,[si+2]	! cx = Sector & head encoded for int $13
  ! bx is correct at $7C00
 endif
retry:
  movb	[$7DFE],#0	! Clear magic for dosemu
  mov	ax,#$0201	! Read 1 sector
  int   $13		! Disk read.
  jnc	sector_loaded

! Error, reset and retry
  xor	ax,ax
  int	$13		! Disk reset

  dec	di
  jnz	retry		! Try again

  mov	si,#disk_read_error
  jmp	no_boot		! Sorry it ain't gonna work.

sector_loaded:
  mov	di,#$7DFE	! End of sector loaded
  cmp	[di],#$AA55	! Check for magic
  jnz	bad_boot	! No? Try next partition.

  mov	bp,si		! LILO says some BBs use bp rather than si
  jmpi	#$7C00,#0	! Go!

! Fatal errors ...........
 if mbrkey
bad_boot:
  mov	si,#no_bootpart
no_boot:		! SI now has pointer to error message
  call	puts
  mov	si,#crlf
  call	puts
tick:
  call	key_wait
  j	tick

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
! int	$19		! This rarely does anything useful...
  jmpi	$0,$FFFF	! Wam! Try or die!

press_key:
  .asciz	"\r\nPress return:"
press_end:
 endif

no_bootpart:
  .asciz	"No active partition"
disk_read_error:
  .asciz	"Disk read error"

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! Instead of loading using the CHS data in the ptbl use the linear addr
!
 if linear
linearise:
  mov	di,#6		! Max retries, int list says 3 ... double it
  mov	dx,[si]		! dh = Drive head, dl = $80 ie HD drive 0
  mov	cx,[si+2]	! cx = Sector & head encoded for int $13
  ! bx is correct at $7C00

  fail! Todo ...
  ret
 endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! Choose the partition based on a pressed key ...

 if mbrkey
key_wait:
  mov	si,#Prompt
  call	puts
  mov	di,#19			! Wait for 18-19 ticks

next_loop:
  mov	ah,#1
  int	$16
  jnz   Got_key
  mov	ah,#0
  int	$1A			! Get current tick
  cmp	dx,si			! If changed DEC our counter.
  jz	next_loop
  mov	si,dx
  dec	di
  jnz	next_loop

  mov	si,#Unprompt		! Nothing has happened, return.
  call	puts

bad_key:
  ret

Got_key:
  cmp	al,#$20
  jnz	not_space
  mov	si,#Pause
  j	showit
not_space:
  mov	Showkey,al
  mov	si,#Showkey
showit:
  call	puts

  mov	ah,#0			! Clean the kbd buffer.
  int	$16

  ! ... Now we use our key ...
  ! 0 		=> Floppy
  ! 1 .. 4	=> Hard disk partition.

  mov	di,#-1
  cmp	al,#$20
  jz	next_loop

  and	ax,#0xF

  jnz	not_floppy
  mov	si,#floppy_part
  br	found_active

not_floppy:
  dec	ax
  test	ax,#0xC
  jnz	bad_key

  mov	cl,#4
  shl	ax,cl
  add	ax,#partition_start
  mov	si,ax

! Set active flag for disk interrupt.
  or	byte [si],#$80

  br	found_active

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
  .asciz	"MBR: "
Unprompt:
  .asciz	"\b\b\b\b\b     \b\b\b\b\b"
Pause:
  .asciz	"\b\b> "
Showkey:
  .ascii	" "
crlf:
  .asciz	"\r\n"
floppy_part:
  .word	0,1

 endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!
 if message
disp_message:
  mov	si,#Banner

 if mbrkey
  br	puts
 else

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
export Banner
Banner:
  .asciz	""
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
! Now make sure this isn't too big!
  if *>table_start
   fail! Partition table overlaps
  endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! The diskman magic number and empty DM partitions.
 if diskman
  org ORGADDR+0xFC
public diskman_magic
diskman_magic:
  .word 0xAA55
  .blkb 12*partition_size-1
  .byte 0
 endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! And finally a copyright message if there's room.
 if *<ORGADDR+0x180
  org ORGADDR+0x180
.asciz "ELKS MBR       "
.asciz "Robert de Bath,"
.asciz "Copyright  1996-2000."
  org partition_start-1
  .byte 0xFF
 endif


!THE END
