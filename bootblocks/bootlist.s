! This bootsector is modified by the installer to have a list of sectors at
! offset 256.

! Table format is a list of dwords. Each dword has a partition offest in
! the low 24 bits and a count in the high.
!
! Prob better to make it:    HH DD MM LL rather than DD HH MM LL
! as this matches the BIOS better
!
! Note:
!	Stack top is at abs location 64k.
!

BOOTSEG = 0x07c0
LOADSEG = 0x07e0	! Just after boot sector.

public linear
linear = 1		! Is linear processing done ?

public floppy
floppy = 0 *linear	! Allow for floppy drive ?
public reloc
reloc  = 1 *linear	! Auto configure of bootpart.

macro locn
  if *-start>?1
   fail
  endif
  .blkb	?1 + start-*
mend

use16
entry start

org $7b00

! temporary space

n_cyl:		.blkw	1
n_secs:		.blkw	1
t_drive:	.blkb	1
t_sector:	.blkb	1
t_secs:		.blkw	1

org $7c00
start:
  j	code

runit:
  jmpi	0,0		! This instruction is altered by the installer

public execaddr		! Via this label.
execaddr = *-4

public bootpart
bootpart:
  .data4	0x80000011
code:

  if	reloc
  mov	cx,[si+8]	! Fetch the linear address of part from DS:SI
  mov	dh,[si+10]	! DL is drive number
  xchg	dl,dh		! Into normal form.
  endif

  xor	ax,ax
  mov	ds,ax
  mov	ss,ax
  mov	sp,ax

  if	reloc
  mov	[bootpart],cx
  mov	[bootpart+2],dx
  endif

! ASSUME ds=ss= 0  (cs should also be zero)

  mov	ax,#LOADSEG
  mov	es,ax

  mov	si,#table
load_addrs:
  cld

 if	linear
  lodsw
  mov	cx,ax
  add	cx,[bootpart]
  lodsw
  mov	dx,[bootpart+2]
  adc	dl,al
  mov	al,ah
 else
  lodsw
  mov	cx,ax
  lodsw
  mov	dx,ax
  lodsb
 endif

  test	al,al
  jz	runit

loadem:
  xor	bx,bx
  push	ax
  call	cread
  jc	failed
  pop	ax
  mov	cl,#5
  sal	ax,cl
  mov	bx,es
  add	ax,bx
  mov	es,ax
  j	load_addrs

! Humm, something goes wrong, not much we can do ...
! Let`s squeak then try again
failed:
  mov	ax,#$0E45
  mov	bx,#7
  int	$10
  pop	ax

  j	loadem

cread:
	! This function is like BIOS 1302 but it`s linear.
	! It's taken, almost, intact from LILO.

	! DH is drive, DL:CX is linear address, AL is count.
	! ES:BX is load address, beware of DMA limitations

	! All registers except AX and flags are preserved.

	mov	t_secs,al	! save number of sectors
lnread:	push	cx		! keep linear address
	push	dx
	xchg	dl,dh

	if	linear
	if	floppy
	test	dl,#0x80	! Floppy is physical only.
	jz	lrd
	endif

	push	bx		! BX is used as scratch
	push	cx		! LSW
	push	dx		! MSW with drive
	mov	ah,#8		! get drive geometry (do not clobber ES:DI)
	push	es
	push	di
	int	0x13
	pop	di
	pop	es
	mov	bl,dh		! BL <- #heads
	pop	dx		! get MSW

	jnc	int_ok		! error -> quit

	pop	cx		! discard stack contents
	pop	bx
	pop	dx
	pop	cx
	ret			! (carry is already set)
int_ok:
	mov	t_drive,dl	! save drive
	mov	dl,dh		! linear address into DX:AX
	xor	dh,dh
	mov	bh,dh		! (clear BH too)
	pop	ax
	push	cx		! compute #cyls-1
	xchg	ch,cl
	rol	ch,1
	rol	ch,1
	and	ch,#3
	mov	n_cyl,cx	! save #cyls-1
	pop	cx
	and	cx,#0x3f	! CX <- #secs
	mov	n_secs,cx
	div	cx		! AX <- track, DX <- sector
	inc	dl
	mov	t_sector,dl
	xor	dx,dx		! divide by #heads
	inc	bx
	div	bx		! AX <- cylinder, DX <- head
	mov	dh,dl		! set up DX (head:drive)
	mov	dl,t_drive
	cmp	ax,n_cyl	! valid cylinder number ?
	ja	linerr3		! no -> error
	xchg	ah,al		! build cylinder number
	ror	al,1
	ror	al,1
	or	al,t_sector
	mov	cx,ax
	pop	bx		! restore BX
	and	ax,#0x3f	! read beyond end of track ?
	add	ax,t_secs
	cmp	ax,n_secs
	jna	intrk		! no -> go on
	mov	al,n_secs	! read to end of track
	sub	al,t_sector
	inc	al
	jmp	lrd		! read it
intrk:	mov	al,t_secs	! read all sectors

	endif

lrd:	push	ax		! save AX and BX
	push	bx

	mov	ah,#2		!
	int	0x13		! read sectors

	pop	bx		! restore AX and BX and the linear address
	pop	ax
	pop	dx
	pop	cx

	if	linear

	jc	linerr		! error -> quit
	sub	t_secs,al	! adjust sector count
	jz	lindone		! zero -> done
	xor	ah,ah		! increase linear address
	add	cx,ax
	adc	dh,#0
	xchg	ah,al		! move BX
	add	ah,ah
	add	bx,ax
	jc	interr		! crossing segment boundaries -> error
	br	lnread		! process remaining sectors

linerr3:pop	bx		! pop BX and linear address
	pop	dx
	pop	cx
interr:	xor	ax,ax		! zero indicates internal error

	else
	jnc	lindone
	endif

linerr:	stc			! error
	ret
lindone:clc			! no error
	ret			! done


!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

public table
table:
! .long	0x80000012
! .byte 1
! .long 0x07e00000
! .byte 0
