/* Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <sys/times.h>

/* MSDOS has it's own versions */
#ifndef __MSDOS__
#ifdef __AS386_32__

/********************** Function __cstartup *******************************/

#ifdef L___cstart3

void (*__cleanup)() = 0;
char ** environ;

#asm
  loc	2
call_main:
  .long _main		! Segment 2 is the trailing pointers, main and the
  .long	call_exit	! routine to call exit.
#if __FIRST_ARG_IN_AX__
  .data
saved_arg1:
  .long 0
#endif
#if __CALLER_SAVES__
  .data
loopy_safe:
  .long 0
#endif
  .text

export ___mkargv
___mkargv:		! BCC Tells linker to init argv ... none needed.

export ___cstartup
___cstartup:		! Crt0 startup (Linux style)
  mov	eax,[esp]
  test	eax,eax
  jz	call_exit	! If argc == 0 this is being called by ldd, exit.
  mov	eax,[esp+8]
  mov	[_environ],eax
#if __FIRST_ARG_IN_AX__
  pop	[saved_arg1]	! Argc will go into eax
#endif

  mov	esi,#auto_start	! Pointer to first autostart function
auto_run:
#if __FIRST_ARG_IN_AX__
  mov	eax,[saved_arg1]
#endif
#if __CALLER_SAVES__
  mov   [loopy_safe],esi
#endif
  mov	ebx,[esi]
  test	ebx,ebx
  jz	no_func
  call	ebx		! Call the function
no_func:
#if __CALLER_SAVES__
  mov   esi,[loopy_safe]
#endif
  add	esi,#4 		! SI at next
  jmp	auto_run	! And round for the next.

call_exit:		! Last item called by above.
  pop	ebx		! Be tidy.
#if !__FIRST_ARG_IN_AX__
  push	eax		! At the end the last called was main() push it`s
#endif
  call	_exit		! return val and call exit();
bad_exit:
  jmp	bad_exit	! Exit returned !!

export _exit
export __exit
_exit:			! exit(rv) function
#if __FIRST_ARG_IN_AX__
  mov	[saved_arg1],eax
#else
  push	[esp+4]		! Copy the `rv` for the exit fuctions.
#endif
  mov	ebx,[___cleanup] ! Call exit, normally this is `__do_exit`
  test	ebx,ebx
  je	no_clean	! But it`s default is null
  call	ebx
no_clean:
#if __FIRST_ARG_IN_AX__
  mov	eax,[saved_arg1]
#else
  add	esp,#4
#endif
__exit:			! _exit(rv)
  br	___exit		! This is just an alias for __exit();

#endasm
#endif

/********************** Function ? ************************************/

/*---*/
#endif
#endif
