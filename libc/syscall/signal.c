
#ifndef __MSDOS__
#ifdef __AS386_16__

#include <errno.h>
#include <signal.h>

typedef __sighandler_t Sig;

extern int __signal __P((int, __sighandler_t));
static Sig system_signal();

Sig __sigtable[_NSIG-1];

/*
 * Signal handler.
 *
 */

/*
 * KERNEL INTERFACE:
 *   It is assumed the kernel will never give us a signal we haven't
 *   _explicitly_ asked for!
 *
 * The Kernel need only save space for _one_ function pointer
 * (to system_signal) and must deal with SIG_DFL and SIG_IGN
 * in kernel space.
 *
 * When a signal is required the kernel must set all the registers as if
 * returning from a interrupt normally then push the number of the signal
 * to be generated, push the current pc value, then set the pc to the
 * address of the 'system_signal' function.
 */

Sig
signal(number, pointer)
int number;
Sig pointer;
{
   Sig old_sig;
   int rv;
   if( number < 1 || number >= _NSIG ) { errno=EINVAL; return SIG_ERR; }

   if( pointer == SIG_DFL || pointer == SIG_IGN )
      rv = __signal(number, pointer);
   else
      rv = __signal(number, (__sighandler_t) system_signal);

   if( rv < 0 ) return SIG_ERR;

   old_sig = __sigtable[number-1];
   __sigtable[number-1] = pointer;

   switch(rv)
   {
   case 0: return SIG_DFL;
   case 1: return SIG_IGN;
   return old_sig;
   }
}

#asm
  .text
_system_signal:		! When this is called by the kernel the stack contains
  pushf			! in order:
  push	ax		! 
  push	bx		!    The signal number,			(NOS)
  push	cx		!    The program counter,		(TOS)
  push	dx		!
  push	si		! It does NOT contain the CS register or the flags.
  push	di		! This means it cannot be unraveled by an iret.
  push	bp
  push	es		! Note also only ES segment register is saved.
  mov	bx,sp		! Unlike minix the rv from a system call is in AX.
  mov	bx,[bx+20]
#if __FIRST_ARG_IN_AX__
  mov	ax,bx
#else
  push	bx		! NB this is _unchecked_, do we want to ?
#endif
  add	bx,bx
  mov	bx,[bx+___sigtable-2]	! Offset by 2 cause no entry for signal 0
  call	bx		! Do we want to check BX for 0 or 1 ?
  inc	sp
  inc	sp
  pop	es
  pop	bp
  pop	di
  pop	si
  pop	dx
  pop	cx
  pop	bx
  pop	ax
  popf
  ret	#2		! Get rid of the signum too.
#endasm

#endif /* __AS386_16__ */
#endif /* __MSDOS__ */
