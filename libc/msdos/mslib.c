/* Copyright (C) 1998 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#if !__FIRST_ARG_IN_AX__
#ifdef __AS386_16__
#ifdef __MSDOS__

#include <dos.h>
#include <fcntl.h>
#include <errno.h>
int errno;

#ifdef L_dos_chdir
chdir(path)
char * path;
{
#asm
  mov	bx,sp
  mov	bx,_chdir.path[bx]
  cmp	[bx+1],':
  jnz	do_chdir
  mov	dl,[bx]
  and	dl,#$3F
  dec	dl
  mov	ah,#$0E
  int	$21
do_chdir:
  mov	ah,#$3B
  mov	dx,bx
do_dxop:
  int	$21
  jnc	op_ok
  br	exterror
op_ok:
  xor	ax,ax
#endasm
}
#endif

#ifdef L_mkdir
mkdir(path)
char * path;
{
#asm
  mov	bx,sp
  mov	dx,_mkdir.path[bx]
  mov	ah,#$39
  j	do_dxop
#endasm
}
#endif

#ifdef L_rmdir
rmdir(path)
char * path;
{
#asm
  mov	bx,sp
  mov	dx,_rmdir.path[bx]
  mov	ah,#$3A
  j	do_dxop
#endasm
}
#endif

#ifdef L_unlink
unlink(path)
char * path;
{
#asm
  mov	bx,sp
  mov	dx,_unlink.path[bx]
  mov	ah,#$41
  xor	cx,cx		! Attrib 0 for servers.
  j	do_dxop
#endasm
}
#endif

/*************************************************************************
  TO TEST ...  
*/

#ifdef L_sleep
sleep(secs)
unsigned secs;
{
   int counter = 0;
   int es = __get_es();

   __set_es(0x40);

   while(secs>0)
   {
      int c = __peek_es(0x6C);
      while( c == __peek_es(0x6C) ) ;

      counter += 549;
      if( counter > 10000 )
      {
	 secs--;
	 counter -= 10000;
      }

      /* Interrupt on ctrl-break */
      if( __peek_es(0x71) & 0x80 ) break;
   }
   __set_es(es);

   return secs;
}
#endif

/*************************************************************************
  TODO ...  
*/

#ifdef L_dos_access
access(filename, amode)
char * filename;
int amode;
{
   XXX;
}
#endif

#ifdef L__dos_allocmem
_dos_allocmem(size ...)
{

}
#endif

#ifdef L__chmod
_chmod(path, func, attrib)
char * path;
int func, attrib;
{
   XXX;
}
#endif

#ifdef L_chmod
chmod(path, mode)
char * path;
int mode;
{
   XXX;
}
#endif

#ifdef L_clock
clock_t clock()
{
   XXX;
}
#endif

#ifdef L_ctrlbrk
ctrlbrk(func)
int (*func)();
{
   XXX;
}
#endif

#ifdef L_delay
delay(millis)
unsigned millis;
{
   XXX;
}
#endif

#ifdef L_dosexterr
dosexterr(ptr)
struct DOSERROR * ptr;
{
   XXX;
}
#endif

/*
#ifdef __dos_getdiskfree
*/

#ifdef L_dup
dup(fd)
int fd;
{
   XXX;
}
#endif

#ifdef L_dup2
dup2(ofd, nfd)
int ofd, nfd;
{
   XXX;
}
#endif

#ifdef L_filelength
long
filelength(fd)
int fd;
{
   XXX;
}
#endif

#ifdef L_findfirst
findfirst(path, fcb, attrib)
char * path;
void * fcb;
int attrib;
{
   XXX;
}
#endif

#ifdef L_findnext
findnext(fcb)
void * fcb;
{
   XXX;
}
#endif

#ifdef L_stat
stat(path, st)
char * path;
struct stat * st;
{
   XXX;
}
#endif

#ifdef L_fstat
fstat(fd, st)
int fd;
struct stat * st;
{
   XXX;
}
#endif

#ifdef L_getcbrk
getcbrk()
{
   XXX int 0x21/33;
}
#endif

#ifdef L_setcbrk
setcbrk(mode)
int mode;
{
   XXX int 0x21/33;
}
#endif

#ifdef L_getcwd
getcwd(buf, buflen)
char * buf;
int buflen;
{
   XXX;
}
#endif

#ifdef L__getdrive
_getdrive()
{
   XXX;
}
#endif

#ifdef L_getenv
char *
getenv(varname)
char * varname;
{
   XXX;
}
#endif

#ifdef L_putenv
putenv(varname)
char * varname;
{
   XXX;
}
#endif

#ifdef L_getpid
getpid()
{
   XXX;
}
#endif

#ifdef L_rename
rename(oldname, newpath)
char *oldpath, *newpath;
{
   XXX;
}
#endif

#ifdef L_setmode
setmode(fd, amode)
int fd, amode;
{
   XXX;
}
#endif


#ifdef L_system
system(cmd)
char * cmd;
{
   XXX;
}
#endif

#ifdef L_umask
umask(mode)
int mode;
{
   XXX;
   /*
    * save umask value for open() use it for read-only bit.
    */
}
#endif

#ifdef L_utime
utime(path, times)
char * path;
stuct utimbuf * times;
{
   XXX;
}
#endif

#endif /* __MSDOS__ */
#endif /* __AS386_16__ */
#endif /* !__FIRST_ARG_IN_AX__ */
