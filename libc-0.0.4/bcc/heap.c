/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/****************************************************************************/

#ifdef L_errno
int errno = 0;	/* libc error value */
#endif

/****************************************************************************/

#ifdef L___brk_addr
#ifdef __AS386_16__
#asm
.data
export brk_addr
brk_addr: .word __end	! This holds the current return for sbrk(0)
.text
#endasm
#endif
#endif

/****************************************************************************/

#ifdef L_sbrk
#ifdef __AS386_16__
int sbrk(brk_off)
unsigned int brk_off;
{
#asm
  mov	bx,sp
#if !__FIRST_ARG_IN_AX__
  mov	ax,[bx+2]	! Fetch the requested value
#endif
  test	ax,ax
  jnz	has_change
  mov	ax,[brk_addr]	! Simple one; read current - can`t fail.
  jmp	eof

has_change:
  js	go_down
  add	ax,[brk_addr]	! Goin up!
  jc	Enomem
  sub	bx,#64		! Safety space
  cmp	bx,ax		! Too close ?
  jb	Enomem

sbrk_ok:
#ifndef __MSDOS__
  push	ax 		! MSDOS `kernel` doesn`t care
  call	___brk		! Tell the kernel
  test	ax,ax
  pop	ax		! ASSUME ___brk doesn`t alter stack!
  jnz	Enomem		! Ugh! kernel didn`t like the idea!
#endif
  xchg	[brk_addr],ax	! Save away new val
  jmp	eof		! Return it
go_down:
  add	ax,[brk_addr]
  jnc	Enomem
  cmp	ax,#__end
  jae	sbrk_ok

Enomem:
  mov	ax,#12		! This should be ENOMEM not a magic.
  mov	[_errno],ax
  mov	ax,#-1
eof:
#endasm
}
#endif
#endif

/****************************************************************************/

#ifdef L_brk
#ifdef __AS386_16__
int
brk(new_brk)
void * new_brk;
{
#asm
  mov	bx,sp
#if !__FIRST_ARG_IN_AX__
  mov	ax,[bx+2]	! Fetch the requested value
#endif
  sub	bx,#64		! Safety space
  cmp	bx,ax		! Too close ?
  jb	Enomem
  cmp	ax,#__end
  jae	brk_ok
Enomem:
  mov	ax,#12		! This should be ENOMEM not a magic.
  mov	[_errno],ax
  mov	ax,#-1
  ret
brk_ok:
#ifndef __MSDOS__
  push	ax
  call	___brk		! Tell the kernel
  test	ax,ax
  pop	bx		! ASSUME ___brk doesn`t alter stack!
  jnz	Enomem		! Ugh! kernel didn`t like the idea!
  mov	[brk_addr],bx	! Save away new val
#else
  mov	[brk_addr],ax	! MSDOS `kernel` doesn`t care
  mov	ax,#0
#endif
#endasm
}
#endif
#endif

/****************************************************************************/
