
#if !__FIRST_ARG_IN_AX__
#ifdef __AS386_16__

#include <bios.h>
#include <errno.h>

#ifdef L_bios_disk_rd
_bios_disk_read(drive, cyl, head, sect, length, buffer)
{
#asm
  push	bp
  mov	bp,sp

  push	es
  push	ds
  pop	es

  mov	dl,[bp+2+__bios_disk_read.drive]
  mov	ch,[bp+2+__bios_disk_read.cyl]
  mov	dh,[bp+2+__bios_disk_read.head]
  mov	bx,[bp+2+__bios_disk_read.buffer]

#if 0
  mov	ax,[bp+2+__bios_disk_read.cyl]	! Bits 10-11 of cylinder, AMI BIOS.
  mov	cl,#4
  sar	ax,cl
  and	al,#$C0
  xor	dh,al
#endif

  mov	cl,[bp+2+__bios_disk_read.sect]
  and	cl,#$3F
  mov	ax,[bp+2+__bios_disk_read.cyl]	! Bits 8-9 of cylinder.
  sar	ax,#1
  sar	ax,#1
  and	al,#$C0
  or	cl,al

  mov	al,[bp+2+__bios_disk_read.length]
  mov	ah,#$02
  int	$13
  jc	read_err1
  mov	ax,#0
read_err1:
  xchg	ah,al
  xor	ah,ah

  pop	es
  pop	bp
#endasm
}
#endif

#ifdef L_bios_disk_wr
_bios_disk_write(drive, cyl, head, sect, length, buffer)
{
#asm
  push	bp
  mov	bp,sp

  push	es
  push	ds
  pop	es

  mov	dl,[bp+2+__bios_disk_write.drive]
  mov	ch,[bp+2+__bios_disk_write.cyl]
  mov	dh,[bp+2+__bios_disk_write.head]
  mov	bx,[bp+2+__bios_disk_write.buffer]

#if 0
  mov	ax,[bp+2+__bios_disk_write.cyl]	! Bits 10-11 of cylinder, AMI BIOS.
  mov	cl,#4
  sar	ax,cl
  and	al,#$C0
  xor	dh,al
#endif

  mov	cl,[bp+2+__bios_disk_write.sect]
  and	cl,#$3F
  mov	ax,[bp+2+__bios_disk_write.cyl]	! Bits 8-9 of cylinder.
  sar	ax,#1
  sar	ax,#1
  and	al,#$C0
  or	cl,al

  mov	al,[bp+2+__bios_disk_write.length]
  mov	ah,#$03
  int	$13
  jc	read_err2
  mov	ax,#0
read_err2:
  xchg	ah,al
  xor	ah,ah

  pop	es
  pop	bp
#endasm
}
#endif

#ifdef L_bios_get_dpt
long
_bios_get_dpt(drive)
{
#asm
  push	bp
  mov	bp,sp

  push	di
  push	es

  mov	dl,[bp+2+__bios_get_dpt.drive]

  mov	ah,#$08
  int	$13
  jnc	func_ok
  mov	cx,ax
  mov	dx,#-1
func_ok:
  mov	ax,cx

  pop	es
  pop	di
  pop	bp
#endasm
}
#endif

#ifdef L_bios_disk_rs
_bios_disk_reset(drive)
{
#asm
  push	bp
  mov	bp,sp

  push	di
  push	es

  mov	dl,[bp+2+__bios_disk_reset.drive]

  mov	ah,#$08
  int	$13
  jnc	reset_ok
  mov	cx,ax
  mov	dx,#-1
reset_ok:
  mov	ax,cx

  pop	es
  pop	di
  pop	bp
#endasm
}
#endif

#endif
#endif
