/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#if defined(__AS386_16__) || defined(__AS386_32__)
#define CRT0_OK
/*
 * Change of aim here, to allow easy alteration for generation of COM files
 * this code has been moved into libc. This means only the linker needs to
 * to be told to fetch a different libc the crt0 is unaltered.
 *
 * This also allows us to deal with __FIRST_ARG_IN_AX__ and __CALLER_SAVES__
 * in the same way.
 */

#asm
.text
entry startup		! Crt0 startup
startup:
  br	___cstartup
export no_op
no_op:			! Generic no operation call
  ret

  .ascii __LIBC_VER__	! Version id.

  loc	1		! Segment 1 is where the pointers to the autostart
			! functions are stored.
#if !__AS386_32__
ZP_safety:
  .word	0		! But first some zeros to avoid null pointer writes.
  .word	0
  .word	0
  .word	0
#endif
export auto_start
auto_start:

#endasm
#endif /* __AS386_16__ or __AS386_32__ */

#if defined(__GNUC__) && defined(__i386__)
#ifdef __ELF__
#define CRT0_OK
__asm__(".globl	_start\n_start:");
__asm__("jmp	__cstartup");
__asm__(".globl	__no_op\n__no_op:");
__asm__("ret");
#else

#error This library is for ELF only, sorry.
__asm__(".globl	startup\nstartup:");
__asm__("jmp	___cstartup");
__asm__(".globl	no_op\nno_op:");
__asm__("ret");
#endif

#endif /* defined(__GNUC__) && defined(__i386__) */

#ifndef CRT0_OK
#error You are not going to get far without crt0.o!
#endif
