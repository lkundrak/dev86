!
! This is floppy bootblock to load the first item from a tar file.
!
! The tar file is constructed with a GNU TAR label at the start
! which is replaced with this by the makeboot.c program.
!
! The first item must be a Linux-8086 executable compiled with the -Ms option
!
! Copyright (C) 1996 Robert de Bath, distributed under the GPL Version 2.
!

BOOTADDR= 0x7C00	! The address that the boot seg is loaded.

RELADDR = 0x0600	! This is floppy, so we can relocate to 'super' address
			! It appears the 0500 is also ok as that's DOS memory
			! But then where can we get 200 bytes of stack!

STACK   = 0x0600	! Inital stack, 0 makes bad_magic

LOADSEG = 0x0080	! Segment for program load, this must be 512b aligned
			! The first 32 bytes are the a.out header.
DEBUG=1  		! Print dots ...

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

macro locn
  if *-start>?1
   fail! ?1
  endif
  .blkb	?1 + start-*
mend

! On startup from a floppy boot it`s probably best if we assume _nothing_
! that we don`t have to ...
!
! So, `start` is, erm, right here :-)
! The boot block is re-loaded from the floppy for relocation.
!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  use16
  org	RELADDR

start:
  ! Don't even _think_ of changing this unless you're 110% sure!
  .byte 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
  .byte 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90

  block start
  .asciz "ENIAC"
  .byte $C0
  endb

  ! All right, I'll help, you can use any of these chars:
  !     "/?@ABCDEFGHIJKLMNO"
  ! Others are likely to cause ... problems.

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

! Beware; the assumption is this code is relocatable.

  xor	ax,ax
  mov	ds,ax
  mov	es,ax
  mov	ss,ax
 if STACK = 0
  mov	sp,ax
 else
  mov	sp,#STACK		! Stack location
 endif
  cld

! ASSUME ds=ss=es=cs=0
keep_trying1:
! Get the tar block after this program, and relocate this code.
  mov	ax,#$0202		! Read 2 sectors
  mov	bx,#start		! Where this _should_ be
  mov	cx,#$0001		! From sector 1
  xor	dx,dx			! Of the floppy drive head zero
  int	$13
  jc	keep_trying1
  jmpi	going,#0
going:

! OK. Code will now be run at correct address.

if DEBUG
  call	pboot
endif

  mov	al,tar_link
  cmp	al,#'0
  jnz	nogood			! Make sure next item is file.

  call	probe_floppy

  call	get_size		! Get number of clicks in di
  test	di,di
  jz	nogood			! If it`s zero .. Hmm

 if STACK = 0
  mov 	sp,#overstack		! Real bad magic :-)
 endif

  call	load_sectors

! This requires that we have been relocated, so we can't call it too soon.
nogood:
  push	cs
  pop	es

hcode:
  mov	ax,#$0201		! Read 1 sector
  mov	bx,#BOOTADDR		! into the boot area
  mov	cx,#1			! sector 1
  mov	dx,#$0080		! hard drive head zero
  int	$13
  jc	hcode			! Keep trying forever!
  jmpi	BOOTADDR,#0

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!
! These are the number of sectors per track that will be scanned for.
! For 3.5 inch floppies 36 is 2.88 Mb, 18 is 1.44Mb, 21 is 1.68Mb on
! a 1.44Mb floppy drive. 15 and 9 are for 5.25 inch floppies.

disksizes: .byte 36,32,21,18,15,9

! End of part 1
!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! This is important bits of the TAR Block.
! The chksum field will need to be changed if any mods are done to this
! file so that the bare header is a TAR file. If it's installed using
! makeboot that program will correct the sum as it also changes the time.

  locn(100)
blk_mode:	.asciz  "   4111"
blk_uid:	.blkb	8
blk_gid:	.blkb	8
blk_size:	.blkb	12
blk_mtime:	.asciz	"6141567743 "
if DEBUG
blk_chksum:	.asciz	" 142273"
else
blk_chksum:	.asciz	" 127270"
endif
blk_link:	.byte	'V

! Sneaks here, overlay zero init vars on tar data.

  block	blk_mode
public sectors
sectors:	.blkw 1			! 
bios_disk:	.blkb 12		! BPB for FD0
head:		.word 0			! current head
track:		.word 0			! current track

  endb

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!
! This code taken from the Linux bootsector and adjusted to suit
!
!	Many BIOS's default disk parameter tables will not 
!	recognize multi-sector reads beyond the maximum sector number
!	specified in the default diskette parameter tables - this may
!	mean 7 sectors in some cases.
!
!	Since single sector reads are slow and out of the question,
!	we must take care of this by creating new parameter tables
!	(for the first disk) in RAM.  We will set the maximum sector
!	count to 36 - the most we will encounter on an ED 2.88.  
!
!	High doesn't hurt.  Low does.
!
!	Segments are as follows: ds=es=ss=cs=0
!

probe_floppy:
	mov	di,#bios_disk

	mov	bx,#0x78		
! 0:bx is parameter table address
	push	ds
	lds	si,[bx]

! ds:si is source

	mov	cl,#6			
! copy 12 bytes
	cld
	push	di

	rep
	movsw

	pop	di
	pop	ds
	movb	4[di],*36		! patch sector count

	mov	[bx],di
	mov	2[bx],es

! It seems that there is no BIOS call to get the number of sectors.  Guess
! 36 sectors if sector 36 can be read, 18 sectors if sector 18 can be read,
! 15 if sector 15 can be read.  Otherwise guess 9.

	mov	si,#disksizes		! table of sizes to try

probe_loop:
	lodsb
	cbw				! extend to word
	mov	sectors, ax
	cmp	al,#9
	je	got_sectors		! if all else fails, try 9
	xchg	ax, cx			! cx = track and sector
	xor	dx, dx			! drive 0, head 0
	mov	bx,#probe_buf 		! address after setup (es = cs)
	mov	ax,#0x0201		! service 2, 1 sector
	int	0x13
	jc	probe_loop		! try next value
got_sectors:

	ret

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!
! This function reads the size of the first file in the TAR image. It's
! stored in octal in the tar header that was loaded just after this block.

get_size:
  mov	di,#tar_size
  xor	dx,dx
  xor	bx,bx
  xor	cx,cx

next_dig:
  mov	al,[di]
  and	al,#$F8
  cmp	al,#'0
  jne	no_dig

  mov	al,[di]
  and	al,#$07

  mov	cl,#3
mul_loop:
  sal	bx,#1		! Double prec * 8
  rcl	dx,#1
  loop	mul_loop
  or	bl,al

no_dig:
  inc	di
  cmp	di,#tar_mtime
  jne	next_dig

  add	bx,#15		! How many clicks ?
  adc	dx,cx		! AKA zero
  mov	cl,#4
div_loop:
  shr	dx,#1
  rcr	bx,#1
  loop	div_loop

  mov	di,bx
  ret

overstack:

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!
! This is good so far, is it an 8086 a.out file ?
!  If so start it.

go_go_go:
  xor	dx,dx		! DX=0 => floppy drive
  push	dx		! CX=0 => partition offset = 0
  mov	si,[sectors]	! Save for monitor.out

  mov	bx,#LOADSEG
  mov	ds,bx		! DS = loadaddress
  xor	di,di		! Zero
  mov	ax,[di]
  cmp	ax,#0x0301	! Right magic ?
  jnz	bad_magic	! Yuk ...
  mov	ax,[di+2]
  and	ax,#$20		! Is it split I/D ?
  jz	impure		! No ...
  mov	cl,#4
  mov	ax,[di+8]
  shr	ax,cl
impure:
  pop	cx		! Partition offset.
  inc	bx
  inc	bx		! bx = initial CS
  add	ax,bx
  mov	ss,ax
  mov	sp,[di+24]	! Chmem value
  mov	ds,ax

  ! AX=ds, BX=cs, CX=X, DX=X, SI=X, DI=0, BP=X, ES=X, DS=*, SS=*, CS=*

bad_magic:
  push	bx		! jmpi	0,#LOADSEG+2
  push	di
  retf

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! This isn't in a normal volume lable, no problem it leaves more space.
!
! locn(257)
! blk_magic:	.ascii	"ustar  "
! 		.blkb	1
! blk_uname:	.ascii	"TarBoot/Copyright"
! blk_uname_end:
! 		.blkb	32+blk_uname-blk_uname_end
! blk_gname:	.ascii	"R de Bath"
! blk_gname_end:
!  		.blkb	32+blk_gname-blk_gname_end
! blk_devmaj:	.blkb	8
! blk_devmin:	.blkb	8
!
!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

if DEBUG
dot:	mov	al,#'.
putc:
	push	bx
	mov	bx,#7
	mov	ah,#$E
	int	$10
	pop	bx
	ret

endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!
! This code taken from the Linux bootsector and adjusted to suit
!
! This routine loads the system at address LOADSEG, making sure
! no 64kB boundaries are crossed. We try to load it as fast as
! possible, loading whole tracks whenever we can.
!
! LOADSEG must be sector aligned and sectors will be loaded upto 
! the next 64k boundry past the segment specified by DI.

load_sectors:
	if LOADSEG & $F000 = 0
	xor	ax,ax
	else
	mov	ax,#LOADSEG & $F000	! segment to load to
	endif
	mov	es,ax

! dx=sectors read of current track
! bx=offset in memory of block
! si=sectors read to block
! al=number to read

read_it:
	mov	dx,#2		! Volume lable + tar header
	if LOADSEG & $0FFF = 0
	xor	si,si
	else
	mov	si,#(LOADSEG & $FFF)>>5
	endif
	add	di,#LOADSEG
rp_read:
if DEBUG
        call	dot
endif
	mov ax,es
	cmp ax,di		! have we loaded all yet?
	jbe ok1_read
	jmp	go_go_go	! Loaded, run it.
ok1_read:
	mov ax,sectors		! nr of sectors/track

	sub ax,dx		! remaining of track

	mov cx,ax		! cx= remaining

	add cx,si		! boundary check
	cmp cl,#128
	jbe ok2_read		! 
				! to much-> fill block
	mov ax,#128		! ax=0
	sub ax,si		! so much may be read
	
ok2_read:
	mov bx,si
	mov cx,#9
	shl bx,cl
	call read_track		! do it
	mov cx,ax		! cl=read blocks

	add ax,dx		! ax=new sectors

	cmp ax,sectors		! track done?
	jne ok3_read
	mov ax,#1		! yes
	sub ax,head		
	jne ok4_read		! next head
	inc track		! next track
ok4_read:
	mov head,ax
	xor ax,ax
ok3_read:
	mov dx,ax

	add si,cx
	cmp si,#128		
	jne rp_read

	mov ax,es
	add ah,#0x10
	mov es,ax

	xor si,si
	jmp rp_read

read_track:
	push 	dx
	push 	ax
	push	bx

	mov	cx,dx
	mov	dx,track
	inc	cx
	mov	ch,dl
	mov	dx,head
	mov	dh,dl
	and	dx,#0x0100
	mov	ah,#2
	
	int	0x13			
! ah=02h al=nr sectors to read
! ch=cylinder
! cl=sector
! dh=head
! dl=drive
! es:bx=buffer

	jc	bad_rt
	pop	bx
	pop	ax
	pop	dx
	ret

bad_rt: xor ah,ah
	xor dl,dl
	int 0x13

	pop	bx
	pop	ax
	pop	dx
	jmp read_track

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!
! This is the GNU-TAR 'isextended' field. It must be zero or gtar goes apeshit.
   	locn(482)
   	.byte	0
!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

if DEBUG
pboot:
  mov	si,#blk_load
nextc:
  lodsb
  call	putc
  cmp	al,#0
  jnz	nextc
  ret
endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

! This isn't a hard disk boot sector but give it an HD magic anyway.
locn(510)
  .word 0xAA55

! From here down is where we load stuff.
locn(512)

blk_load:			! Address of block load
tar_name:	.blkb	100
tar_mode:	.blkb	8
tar_uid:	.blkb	8
tar_gid:	.blkb	8
tar_size:	.blkb	12
tar_mtime:	.blkb	12
tar_chksum:	.blkb	8
tar_link:	.blkb	1	! "0"
tar_linkname:	.blkb	100
tar_magic:	.blkb	8	! "ustar  "
tar_uname:	.blkb	32
tar_gname:	.blkb	32
tar_devmaj:	.blkb	8
tar_devmin:	.blkb	8
tar_padd:


  locn(1024)
probe_buf:	.blkb 512


