! This binary is for loading a dev86 a.out file from a floppy without
! a filesystem; to make a bootable disk just do:
!
!  cat boot_fpy.bin monitor.out > /dev/fd0
!

ORGADDR=0x0600
EXEADDR=0x0800		! This must be up to 0x7E0 or 0x0800
LOADSEG=0x0080		! Must be 512b aligned for DMA		

! Padding so you can join with 'cat'.
.org EXEADDR-1
.byte 0

! This marker is needed by many boot managers (and bochs) but the BIOS does
! NOT require it on a floppy.
if EXEADDR=0x0800
.org ORGADDR+0x1FE
.word 0xAA55
endif

.org ORGADDR
entry start
public start
start:
  xor	ax,ax
  mov	si,#$7C00
  mov	di,#ORGADDR

  mov	ss,ax
  mov	sp,di		! Or si or ax

  push	ax
  pop	ds
  push	ax
  pop	es

  mov	cx,#256
  cld
  rep
   movsw
  jmpi	go,#0
go:

! Grrr, have to load sector 1 in by hand.
if EXEADDR=0x0800
Loopi:
  mov	ax,#$0201	! Read 1 sector
  mov	bx,#EXEADDR	! Into EXEADDR
  mov	cx,#$0002	! From sector 2
  xor	dx,dx		! Of the floppy drive head zero
  int	$13
  jc	Loopi
endif

  mov	si,#Boot_message
  call	puts

  mov	ax,[a_text]	! How many sectors to load
  mov	cl,#4
  shr	ax,cl
  mov	bx,ax
  mov	ax,[a_data]
  mov	cl,#4
  shr	ax,cl
  add	ax,bx
  add	ax,#$1F
  mov	cl,#5
  shr	ax,cl		! ax = sectors to read

  ! This routine starts by loading one sector at a time, with most
  ! modern PCs the processor is fast enough to keep up with single
  ! sector reads, in reality an 8Mhz 286 can keep up!
  ! But occasionally some older machines have really poor BIOSes
  ! (Some modern ones too) so once we know how many sectors to read
  ! we switch to reading a track at a time. But we only try it once
  ! for each track. Normally, as long as the load address is sector
  ! aligned, this will work every time but with some BIOSes we can't
  ! read a track without messing with the BPB so if the track read
  ! fails it's one try we fall back to sector reads.
  !
  ! Overall this usually gives good performance, and with a BIOS that
  ! isn't completely broken and correctly formatted floppies will run
  ! at about 2.5 rotations per cylinder (1.25 per track). If you find
  ! your BIOS is one of the bad ones you'll have to format your disks
  ! to a 2:1 interleave.
  !
  ! BTW: It's very easy to make superformat incorrectly calculate the 
  ! inter-sector gaps so it ends up squeezing the sectors to the start 
  ! of the track. This means that only a full track read is fast enough.
  ! I suggest you use fdformat as it always uses 'safe' parameters for 
  ! a 1440k floppy.

  			! AX = count of sectors
  mov	cx,#2		! CX = First sector
  mov	bx,#LOADSEG	! ES:BX = Where to load
  mov	es,bx
  xor	bx,bx		! Initial offset

  xor	dx,dx		! DX = Drive 0

  ! ax=cnt, dl=drv, ch=*, dh=*, cl=sec, es:bx=buffer.

read_data:
  mov	si,ax		! Save big count.
  xor	ch,ch	
  xor	dh,dh

  mov	maxsect,cl	! Save first sector.

load_loop:
  mov	di,#5		! Error retry.

sect_retry:
  mov	ax,#$0201
  ! ah=2, al=1, dl=drv, ch=cyl, dh=head, cl=sec, es:bx=buffer.
  int	$13
  jnc	next_sect

  dec	di		! Retry counter
  jz	sect_error

  cmp	cl,maxsect	! If this is first sector or previously ok sector
  jle	sect_retry	! number then retry.

  mov	maxsect,cl
  j	inc_trk

next_sect:
  mov	ax,es		! Inc load address.
  add	ax,#32
  mov	es,ax

  dec	si		! Had enough ?
  jz	all_loaded

inc_sect:
  inc	cl
  cmp	cl,maxsect
  jnz	load_loop
inc_trk:		! Reached end of track, seek to next.
  mov	cl,#1
  xor	dh,cl
  jnz	load_track
  inc	ch
load_track:
  cmp	si,maxsect	! Is the whole track needed ?
  jb	load_loop	! no, goto load_loop for 1 by 1
  
  ! Try to load the track _once_ only, if it fails go 1 by 1 again.

  mov	ax,maxsect
  dec	ax
  mov	ah,#$02
  ! ah=2, al=*, dl=drv, ch=cyl, dh=head, cl=sec, es:bx=buffer.
  int	$13
  jc	load_loop

  mov	ax,maxsect	! Ok that worked, update the pointers
  dec	ax
  mov	cl,#5
  shl	ax,cl
  mov	di,es
  add	ax,di
  mov	es,ax

  inc	si
  sub	si,maxsect
  jnz	inc_trk

all_loaded:
  ! Now it's loaded turn off the floppy motor.
  mov dx,#0x3f2
  xor al, al
  outb

  ! And start up the program.

  xor	dx,dx		! DX=0 => floppy drive
  push	dx		! CX=0 => partition offset = 0
  mov	si,dx		! Sect/track = 0

  mov	bx,#EXEADDR>>4
  mov	ds,bx		! DS = loadaddress
  xor	di,di		! Zero
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

sect_error:
  ! Disk error, wait then reboot.

  mov	si,#reboot_msg
  call	puts

  xor   ax,ax		! Wait for the user.
  int   $16
  jmpi  $0,$FFFF

puts:
  lodsb
  cmp   al,#0
  jz    EOS
  push  bx
  mov   bx,#7
  mov   ah,#$E                  ! Can't use $13 cause that's AT+ only!
  int   $10
  pop   bx
  jmp   puts
EOS:
  ret

maxsect:
  .word	0

reboot_msg:
  .asciz	"Disk error, press a key to reboot:"

Boot_message:
  .asciz	"Boot sector loaded.\r\n"

! Check for overlap
end_of_code:
  if end_of_code>hitme
     fail! Overlap at end_of_code
  endif

.org EXEADDR
hitme:

magic:		.space 2	! A.out header
btype:		.space 2
headerlen:	.space 4
a_text:		.space 4
a_data:		.space 4
a_bss:		.space 4
a_entry:	.space 4
a_total:	.space 4
a_syms:		.space 4

