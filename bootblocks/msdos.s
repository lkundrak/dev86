!
! This is a bootblock to load an execute a standalone program from an
! MSDOS filesystem on a floppy disk.
!
! The program is divided into two parts, the first 512 bytes contains a
! loader to fetch one block from a file called 'BOOTFILE.SYS' from the
! root directory of the disk. The second 512, which is stored in this file,
! loads the executable using functions supplied by the first.
!
! The 2nd part is loaded as if it's a floppy boot block and can be used to
! store, for instance, a LILO boot block to let LILO boot from an MSDOS
! floppy.
!
! The second part is NOT yet complete!
!
ORGADDR=$0500

use16

! Absolute position macro, fails if code before it is too big.
macro locn
  if *-start>?1
   fail! *-start>?1
  endif
  .blkb	?1 + start-*
mend

org ORGADDR
start:
include sysboot.s

! Data into the temp area, 30 clear bytes
org floppy_temp
root_count:	.blkw	1
old_bpb:	.blkw	2
bios_disk:	.blkb	12

  locn(codestart-start)

  cld			! Assume _nothing_!
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

! Need to fix BPB for fd0 to correct sectors (Like linux bootblock does)
  mov	di,#bios_disk
  mov	bp,#0x78		
! 0:bx is parameter table address
  push	ds
  lds	si,[bp]

! ds:si is source

  mov	cx,#6			
! copy 12 bytes
  push	di
  rep
   movsw
  pop	di
  pop	ds

! New BPB is 0:di
  mov	[bp],di
  mov	2[bp],ax

  mov	al,[dos_spt]
  mov	4[di],al

! For each sector in root dir
! For each dir entry
! If entry name is == boot_name
! then load and call sector

  ! First dir = dos_fatlen*dos_nfat+dos_resv
  mov	ax,[dos_fatlen]
  mov	dl,[dos_nfat]
  xor	dh,dh
  mul	dx
  add	ax,[dos_resv]
  mov	di,ax
  ! DI is sector number of first root dir sector.

  mov	ax,[dos_nroot]
  mov	[root_count],ax
  add	ax,#15
  mov	cl,#4
  shr	ax,cl
  add	ax,di
  mov	bp,ax		! bp is first data sector.

nextsect:
  call	linsect
  mov	ax,#$0201
  int	$13
  jc	floppy_error

! ... foreach dir entry
  mov	si,bx
nextentry:

! test entry name
  push	si
  push	di
  mov	di,#boot_name
  mov	cx,#11
  rep
   cmpsb
  pop	di
  pop	si
  je	got_entry

  add	si,#32
  cmp	si,#512
  jnz	nextentry

  inc	di
  sub	[root_count],#16
  jp	nextsect
  jmp	no_system

got_entry:
  mov	ax,[si+26]	! Cluster number of start of file
  test	ax,ax
  jz	no_system	! Make sure we have a block.

  mov	di,ax		! Save the cluster number we are loading.
  call	linclust

  mov	ax,#$0201
  int	$13
  jc	floppy_error

  mov	bx,#7
  mov	ax,#$0E3E
  int	$10

  jmpi	$7C00,0		! No magics, just go.

! Convert a cluster number into a CHS in CX:DX
linclust:
  sub	ax,#2
  mov	dl,[dos_clust]
  xor	dh,dh
  mul	dx
  add	ax,bp
  mov	di,ax
  ! JMP linsect ...

!
! This function converts a linear sector number in DI 
! into a CHS representation in CX:DX
!
linsect:
  xor	dx,dx
  mov	ax,di
  div	[dos_spt]
  inc	dx
  mov	cl,dl	! Sector num
  xor	dx,dx	! Drive 0
  shr	ax,#1	! Assume dos_heads == 2
  adc	dh,#0	! Head num
  mov	ch,al	! Cylinder
  ret

no_system:
floppy_error:

  mov	si,#error_msg
no_boot:		! SI now has pointer to error message
  lodsb
  cmp	al,#0
  jz	EOS
  mov	bx,#7
  mov	ah,#$E		! Can't use $13 cause that's AT+ only!
  int	$10
  jmp	no_boot
EOS:
  xor	ax,ax
  int	$16
  int	$19		! This should be OK as we haven't touched anything.
  jmpi	$0,$FFFF	! Wam! Try or die!

error_msg:
  .asciz "\r\nError during initial boot\r\nPress a key:"

export boot_name
boot_name:
 .ascii "BOOTFILESYS"
name_end:
!        NNNNNNNNEEE
!
!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!
! Part 2, loaded like a boot block by part 1
!     ... Here we will allow some assumptions (cause we've set them)
fat_table = ORGADDR+$400	! Where to load the fat

locn(512)
part2_addr:
  push	di
  mov	cx,#$100	! Move 256 words
  mov	si,#$7C00	! From default BB
  mov	di,#part2_addr	! To the correct address.
  rep
   movsw
  jmpi	cont2,#0	! Set CS:IP correct.
cont2:

! 1) Load FAT
  ! This assumes all of FAT1 is on the first track, this is normal
  mov	ax,[dos_fatlen]
  mov	ah,#2
  mov	bx,#fat_table
  mov	cx,[dos_resv]	! Fat starts past bootblock
  inc	cx
  xor	dx,dx		! Head zero
  int	$13
  !jc	floppy_error

  mov	bx,#$7C00
  pop	di		! Cluster to start load.

  ! load whole cluster
next_cluster:


  call	next_fat
  cmp	di,#0
  jnz	next_cluster
  br	maincode

next_fat:
  !mov	ax,di
  !mov	bx,ax
  !shr	ax,#1
  !add	bx,ax


! The end ... place a marker.
locn(1023)
  .byte	$FF
maincode:
