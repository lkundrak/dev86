/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/* Alloca is a nice interesting function */
/* Needs change to BCC to work, as BCC doesn't use 'mov sp,bp:pop bp' */
/* The only other way to do it is to use malloc and the emacs alloca */

#if 0
THIS IS BROKEN!!!

#asm
export _alloca
  .text
_alloca:
  pop	cx	! The program counter
  pop	ax	! The argument (>1 arg == snafu)
  inc	ax
  and	al,$FE	! Round up to even

  test	ax,ax
  jz	too_big	! Well _I_ think it means 64k ! :-)     (ANSI too IIUC)

  mov	bx,sp
  sub	bx,ax	! Calc new SP
  jb	too_big	! SP wrapped, snafu!

  mov	ax,[brk_addr]
  add	ax,#64	! Check if brk has allocated the memory first but
  cmp	bx,ax	! leave a little bit of space before we collide!
  jb	too_big

  xchg	bx,sp	! Put it in place, may need the old SP for other args
  mov	ax,sp	! The bottom of the area allocated

		! This is needed if si/di popped before mov sp,bp
  !push	[bx+4]	! This copies the saved register vars and possibly a temp
  !push	[bx+2]	! value, beware snafu if called within complex expression!
  !push	[bx]	! 

  push	ax	! Anything really, it will be deallocated by the caller
  jmp	cx	! Return

too_big:
  xor	ax,ax	! Sorry not enough stack
  push	ax	! More junk
  jmp	cx	! Return
#endasm
#endif
