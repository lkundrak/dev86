/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <sys/types.h>
#include <time.h>

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
  .word _main		! Segment 2 is the trailing pointers, main and the
  .word	call_exit	! routine to call exit.
#if __FIRST_ARG_IN_AX__
  .data
saved_arg1:
  .word 0
#endif
#if __CALLER_SAVES__
  .data
loopy_safe:
  .word 0
#endif
  .text

export ___cstartup
___cstartup:		! Crt0 startup
  pop	cx		! Argc
  mov	bx,sp		! Calculate ptrs to argv and envp
  mov	ax,cx
  inc	ax
  shl	ax,#1
  add	ax,bx
  push	ax		! Push Envp
  mov	[_environ],ax	! And save
  push	bx		! Push argv
#if __FIRST_ARG_IN_AX__
  mov	[saved_arg1],cx
#else
  push	cx		! Push argc
#endif

  mov	si,#auto_start	! Pointer to first autostart function
auto_run:
#if __FIRST_ARG_IN_AX__
  mov	ax,[saved_arg1]
#endif
#if __CALLER_SAVES__
  mov   [loopy_safe],si
#endif
  call	[si]		! Call the function
#if __CALLER_SAVES__
  mov   si,[loopy_safe]
#endif
  inc	si		! SI at next
  inc	si
  jmp	auto_run	! And round for the next.

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

/********************** Function time ************************************/

#ifdef L_time
time_t time(where)
time_t *where;
{
   struct timeval rv;
   if( gettimeofday(&rv, (void*)0) < 0 ) return -1;
   if(where) *where = rv.tv_sec;
   return rv.tv_sec;
}
#endif

/********************** Function lseek ************************************/

#ifdef L_lseek
off_t lseek(fd, posn, where)
int fd;
off_t posn;
int where;
{
   off_t __lseek();
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

/********************** Function abort ************************************/

#ifdef L_abort
#include <signal.h>

int abort()
{
   signal(SIGABRT, SIG_DFL);
   kill(SIGABRT, getpid());	/* Correct one */
   pause();			/* System may just schedule */
   signal(SIGKILL, SIG_DFL);
   kill(SIGKILL, getpid());	/* Can't trap this! */
   __exit(255);			/* WHAT!! */
}
#endif

/********************** Function wait ************************************/

#ifdef L_wait
int
wait(status)
int * status;
{
   return wait4(-1, status, 0, (void*)0);
}
#endif

/********************** Function waitpid ************************************/

#ifdef L_waitpid
int
waitpid(pid, status, opts)
int pid;
int * status;
int opts;
{
   return wait4(pid, status, opts, (void*)0);
}
#endif

/********************** Function sleep ************************************/

#ifdef L_sleep
#include <signal.h>

/* This uses SIGALRM, it does keep the previous alarm call but will lose
 * any alarms that go off during the sleep
 */

static void alrm() { }

unsigned int sleep(seconds)
unsigned int seconds;
{
   void (*last_alarm)();
   unsigned int prev_sec;

   prev_sec = alarm(0);
   if( prev_sec <= seconds ) prev_sec = 1; else prev_sec -= seconds;

   last_alarm = signal(SIGALRM, alrm);
   alarm(seconds);
   pause();
   seconds = alarm(prev_sec);
   signal(SIGALRM, last_alarm);
   return seconds;
}
#if 0
        /* Is this a better way ? If we have select of course :-) */
#include <sys/time.h>
unsigned int
sleep(seconds)
unsigned int seconds;
{
        struct timeval timeout;
	time_t start = time((void*)0);
        timeout.tv_sec = seconds;
        timeout.tv_usec = 0;
        select(1, NULL, NULL, NULL, &timeout);
	return seconds - (time((void*)0) - start);
}
#endif
#endif

/********************** THE END ********************************************/

#endif /* __AS386_16__ */
#endif /* __MSDOS__ */
