/* Copyright (C) 1996 Robert de Bath <robert@mayday.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#if !__FIRST_ARG_IN_AX__
#ifdef __AS386_16__
#ifdef __STANDALONE__

#include <dos.h>
#include <fcntl.h>
#include <errno.h>
int errno;

#ifdef L_bios_start
char ** environ = { 0 };

void (*__cleanup)() = 0;

#asm
  .data
export ___argr
___argr:
  .word 0,0,0,0,0,0,0	! A struct REGS
defarg:
  .word boot_str, 0
boot_str:
  .asciz "boot"

  .text
export ___cstartup	! Crt0 startup
___cstartup:
  mov	___argr+0,ax
  mov	___argr+2,bx
  mov	___argr+4,cx
  mov	___argr+6,dx
  mov	___argr+8,si
  mov	___argr+10,di

zap_bss:		! Clear the BSS
  mov	ax,ds
  mov	es,ax		! ES now data seg
  mov	di,#__edata
  mov	cx,#__end
  sub	cx,di
  xor	ax,ax
  cld
  rep
   stosb

  push	[_environ]
  mov	ax,#defarg	! Don`t define __mkargv, standalone programs don`t
  push	ax		! get any arguments.
  mov	ax,#1
  push	ax

  mov	si,#auto_start	! Pointer to first autostart function
auto_run:
  mov	bx,[si]
  test	bx,bx
  jz	no_entry
  call	bx		! Call the function
no_entry:
  inc	si		! SI at next
  inc	si
  jmp	auto_run	! And round for the next.

call_exit:		! Last item called by above.
  pop	bx		! Be tidy.
  push	ax		! At the end the last called was main() push it`s
  call	_exit		! return val and call exit();
bad_exit:
  jmp	bad_exit	! Exit returned !!

  loc	2
  .word _main		! Segment 2 is the trailing pointers, main and the
  .word	call_exit	! routine to call exit.
data_start:

  .text
export _exit
_exit:			! exit(rv) function
  mov	bx,sp
  push	[bx+2]		! Copy the `rv` for the exit fuctions.
  mov	bx,[___cleanup] ! Call exit, normally this is `__do_exit`
  test	bx,bx
  je	no_clean	! But it`s default is null
  call	bx
no_clean:
  inc	sp
  inc	sp

export __exit
__exit:
  xor	ax,ax
  mov	es,ax
  mov	ax,cs
  seg	es
  mov	[$E6*4+2],ax
  mov	ax,#reti_ins
  seg	es
  mov	[$E6*4],ax
  mov	ax,#$FFFF
  int	$E6		! Try to exit DOSEMU
  			! If we get here we`re not in dosemu.
  seg es
  mov	[$472],#$1234	! Warm reboot.
  jmpi	$0000,$FFFF
reti_ins:
  reti

#endasm

#endif

/****************************************************************************/

#ifdef L___file_3

/* If the block function does track buffering this should be ok ... */
struct {
   int  (*block_rw)();		/* Args (rwoc, &buffer, blockno) 1k blocks */
   				/* 0 = read, 1 = write */
   				/* 2 = open, buffer is fname ptr */
   				/* 3 = close, other args ignored */
   long offset;

   int  flags;
   long block_num;
   char buffer[1024];
} __file_3_data;

#define FILE3_OPEN	1	/* File is open */
#define FILE3_DATA	2	/* buffer has valid contents */
#define FILE3_DIRTY	4	/* buffer has been modified */

#endif

#ifdef L_bios_write
write(fd,buf,len)
int fd,len;
char * buf;
{
   register int v, c;
   if(fd == 1 || fd == 2)
   {
      for(v=len; v>0; v--)
      {
         c= *buf++;
         if( c == '\n') bios_putc('\r');
	 bios_putc(c);
      }
      return len;
   }
   errno = EBADF;
   return -1;
}
#endif

/****************************************************************************/

#ifdef L_bios_read
read(fd,buf,len)
int fd,len;
char * buf;
{
   if(fd == 0) return bios_rdline(buf, len);
   errno = EBADF;
   return -1;
}
#endif

/****************************************************************************/

#ifdef L_bios_lseek
long
lseek(fd, offt, whence)
int fd, whence;
long offt;
{
   if( fd >= 0 && fd <= 2 ) errno = ESPIPE;
   else errno = EBADF;
   return -1L;
}
#endif

/****************************************************************************/

#ifdef L_bios_open
open(name, flags, mode)
char * name;
int flags, mode;
{
   errno = ENOENT;
   return -1;
}
#endif

/****************************************************************************/

#ifdef L_bios_close
close(fd)
int fd;
{
   if( fd >= 0 && fd <= 2 ) errno = ENOSYS;
   else errno = EBADF;
   return -1;
}
#endif

/****************************************************************************/

#ifdef L_bios_isatty
isatty(fd)
int fd;
{
   if( fd >= 0 && fd <= 2 ) return 1;
   return 0;
}
#endif

/****************************************************************************/

#endif
#endif
#endif
