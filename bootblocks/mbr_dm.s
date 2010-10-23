!
! This is a 'Master Boot Record' following the MSDOS 'standards'.
! This BB successfully boots MSDOS, Windows or Linux in CHS or Linear.
!
! NB: This needs as86 0.16.0 or later

! Lowest available is $0500, MSDOS appears to use $0600 ... I wonder why?
ORGADDR=$0500
use512=0	! Put the end marker at byte 510..512
markptab=1	! Put an end marker just below the partition table.

diskman=1	! Disk manager partitions, allows 16 partitions but
		! don't overwrite this with a LILO BB.
revorder=0	! Use physical order for choosing diskman boot partition

linear=0	! Use the linear addresses not the CHS ones (if available)
useCHS=1	! Disable CHS if you need space.
linCHS=0        ! Calculate CHS from linear mbr values.

 if 1&~(useCHS|linear)
  fail! Errm, you can't boot anything without 'linear' or 'useCHS'
 endif

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
export linCHS

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

 if (linear|useCHS)

! Now check the partition table, must use SI as pointer cause that's what the
! partition boot blocks expect.

! If we're using diskman and we're short of space check the partitions in
! physical order. (Order. 4,3,2,1,5,6,7,8,9,10,11,12,13,14,15,16)

 if (diskman&revorder)

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
 if diskman&~(revorder)
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

bad_boot:
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
if linCHS
  call  calc_chs
else
  mov	dx,[si]		! dh = Drive head, dl = $80 ie HD drive 0
  mov	cx,[si+2]	! cx = Sector & head encoded for int $13
  ! bx is correct at $7C00
endif

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

no_boot:		! SI now has pointer to error message
  call	puts
  mov	si,#press_key
  call	puts

keyboot:		! Wait for a key then reboot
  xor	ax,ax
  int	$16
  jmpi	$0,$FFFF	! Reboot.

if useCHS
if linCHS
calc_chs:
  push  bx              ! Save load location

  mov   ah,#8
  int   $13	        ! Drive Geom
  shr   dx,#8
  xchg  ax,dx
  inc   ax              ! AX = No. Heads
  and   cx,#$3f         ! CX = Sectors 
  mul   cx
  xchg  ax,bx           ! BX = .

  mov   ax,[si+8]       ! Linear partition address.
  mov   dx,[si+10]

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

  pop   bx
  ret
endif
endif
!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! 
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

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!

press_key:
  .asciz	"\r\nPress return:"
press_end:

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!

 if (linear|useCHS)
disk_read_error:
  .asciz	"Read error"
no_bootpart:
  .asciz	"Bad partition"
 else
no_bootpart:
  .asciz	"Not a bootable disk"
 endif !(linear|useCHS)

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
! Clear the sector to the bottom of the partition table.
 if markptab
  if *<partition_start-2
  org partition_start-2
  .word 0xAA55
 endif
 endif

 if use512
  org ORGADDR+0x1FE
  .word 0xAA55
 endif

!THE END
