/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <sys/times.h>

/* MSDOS has it's own versions */
#ifndef __MSDOS__
#ifdef __AS386_16__

/********************** Function __cstartup *******************************/

#ifdef L___cstartup

void (*__cleanup)() = 0;
char ** environ;

#asm
  loc	2
call_main:
  .word run_main	! Segment 2 is the trailing pointers, main and the
  .word	call_exit	! routine to call exit.
#if __FIRST_ARG_IN_AX__
  .data
saved_arg1:
  .word 0
#endif
  .data
loop_safe:
  .word 0
  .text

export ___cstartup
___cstartup:		! Crt0 startup
  pop	cx		! Argc
  mov	bx,sp		! Calculate ptrs to argv and envp
  mov	ax,cx
  inc	ax
  shl	ax,#1
  add	ax,bx

export ___mkargv
___mkargv:		! BCC tells the linker to init argc,argv with this.

  push	ax		! Push Envp
  mov	[_environ],ax	! And save
  push	bx		! Push argv
#if __FIRST_ARG_IN_AX__
  mov	[saved_arg1],cx
#else
  push	cx		! Push argc
#endif

  mov	bx,#auto_start	! Pointer to first autostart function
auto_run:
#if __FIRST_ARG_IN_AX__
  mov	ax,[saved_arg1]
#endif
  mov   [loop_safe],bx
  mov	bx,[bx]
  test	bx,bx
  jz	no_entry
  call	bx		! Call the function
no_entry:
  mov   bx,[loop_safe]
  inc	bx		! next
  inc	bx
  jmp	auto_run	! And round for the next.

run_main:
  br	_main

call_exit:		! Last item called by above.
  pop	bx		! Be tidy.
#if !__FIRST_ARG_IN_AX__
  push	ax		! At the end the last called was main() push it`s
#endif
  call	_exit		! return val and call exit();
bad_exit:
  jmp	bad_exit	! Exit returned !!

export _exit
export __exit
_exit:			! exit(rv) function
#if __FIRST_ARG_IN_AX__
  mov	[saved_arg1],ax
#else
  mov	bx,sp
  push	[bx+2]		! Copy the `rv` for the exit fuctions.
#endif
  mov	bx,[___cleanup] ! Call exit, normally this is `__do_exit`
  test	bx,bx
  je	no_clean	! But it`s default is null
  call	bx
no_clean:
#if __FIRST_ARG_IN_AX__
  mov	ax,[saved_arg1]
#else
  inc	sp
  inc	sp
#endif
__exit:			! _exit(rv)
  br	___exit		! This is just an alias for __exit();

#endasm

#endif

/********************** Function lseek ************************************/

#ifdef L_lseek
off_t lseek(fd, posn, where)
int fd;
off_t posn;
int where;
{
   if( __lseek(fd, &posn, where) < 0 ) return -1;
   else return posn;
}
#endif

/********************** Function getpid ************************************/

#ifdef L_getpid
int getpid()
{
   int ppid;
   return __getpid(&ppid);
}
#endif

/********************** Function getppid ************************************/

#ifdef L_getppid
int getppid()
{
   int ppid;
   __getpid(&ppid);
   return ppid;
}
#endif

/********************** Function getuid ************************************/

#ifdef L_getuid
int getuid()
{
   int euid;
   return __getuid(&euid);
}
#endif

/********************** Function geteuid ************************************/

#ifdef L_geteuid
int geteuid()
{
   int euid;
   __getuid(&euid);
   return euid;
}
#endif

/********************** Function getgid ************************************/

#ifdef L_getgid
int getgid()
{
   int egid;
   return __getgid(&egid);
}
#endif

/********************** Function getegid ************************************/

#ifdef L_getegid
int getegid()
{
   int egid;
   __getgid(&egid);
   return egid;
}
#endif

/********************** Function dup2 ************************************/

#ifdef L_dup2

#include <fcntl.h>

int dup2(ifd, ofd)
int ifd;
{
   return fcntl(ifd, F_DUPFD, ofd);
}
#endif

/********************** Function dup ************************************/

#ifdef L_dup
#include <sys/param.h>
#include <fcntl.h>
#include <errno.h>

/* This is horribly complicated, there _must_ be a better way! */

int
dup(fd)
int fd;
{
   int nfd;
   extern int errno;
   int oerr = errno;

   errno = 0;
   for(nfd=0; nfd<NR_OPEN; nfd++)
   {
      if( fcntl(nfd, F_GETFD) < 0 )
         break;
   }
   if( nfd == NR_OPEN ) { errno = EMFILE ; return -1; }
   errno = oerr;
   if( fcntl(fd, F_DUPFD, nfd) < 0 )
   {
      if( errno == EINVAL ) errno = EMFILE;
      return -1;
   }
   return nfd;
}
#endif

/********************** Function getpgrp ************************************/

#ifdef L_getpgrp
int
getpgrp()
{
   return getpgid(0);
}
#endif

/********************** Function times ************************************/

#ifdef L_times
clock_t times(buf)
struct tms* buf;
{
   long rv;
   __times(buf, &rv);
   return rv;
}
#endif

/********************** THE END ********************************************/

#endif /* __AS386_16__ */
#endif /* __MSDOS__ */
