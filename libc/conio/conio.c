/* Copyright (C) 1999 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <conio.h>

/* 
 * I'm not sure if these should be BIOS or dos calls, so I'll assume they're
 * BIOS calls but I may have to do something about Ctrl-C.
 *
 * Not implemented: cgets() cscanf() getpass() ungetch()
 */

#ifdef L_getch
getch()
{
#asm
  xor   ax,ax
  int   $16
#endasm
}
#endif

#ifdef L_getche
getche()
{
   int i = getch();
   if( i & 0xFF ) putch(i);
   return i;
}
#endif

#ifdef L_kbhit
kbhit()
{
#asm
  mov   ah,#1
  int   $16
  jz    nokey
  cmp   ax,#0
  jnz   dort
  mov   ax,#3
dort:
  ret
nokey:
  xor   ax,ax
#endasm
}
#endif

#ifdef L_putch
putch()
{
#asm
#if !__FIRST_ARG_IN_AX__
  mov   bx,sp
  mov   ax,[bx+2]
#endif
  mov   ah,#$0E
  mov   bx,#7
  int   $10
#endasm
}
#endif

#ifdef L_cputs
cputs(str)
char * str;
{
   while(*str) putch(*str++);
}
#endif

#ifdef L_gotoxy
static gotoxy(x,y)
{
#asm
#if __FIRST_ARG_IN_AX__
  mov	bx,sp
  mov	dl,al
  mov	ax,[bx+2]
  mov	dh,al
#else
  mov	bx,sp
  mov	ax,[bx+4]
  mov	dh,al
  mov	ax,[bx+2]
  mov	dl,al
#endif
  mov	ah,#$02
  mov	bx,#7
  int	$10
#endasm
}
#endif

