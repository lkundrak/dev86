/* Copyright (C) 1996 Robert de Bath <robert@mayday.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#if !__FIRST_ARG_IN_AX__
#ifdef __AS386_16__
#ifdef __STANDALONE__

#include <bios.h>
#include <errno.h>
int errno;

#ifdef L_bios_putch
void (*__smart_putch)() = 0;

putch(c)
int c;
{
static int col = 0;
   if (__smart_putch)
      (*__smart_putch)(c);
   else if (c&0xE0) {
      col++;
      asm_putc(c);
   }
   else switch(c)
   {
   case '\t': 
      do putch(' '); while(col&7);
      break;
   case '\n':
      asm_putc('\r');
   case '\r':
      col = 0;
      asm_putc(c);
      break;
   case '\f':
      col = 0;
#asm
      mov ah,#$0F
      int $10
      mov ah,#$00
      int $10
#endasm
      break;
   }
}

static asm_putc(c)
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

/****************************************************************************/

#ifdef L_bios_getch
getch()
{
#asm
  xor	ax,ax
  int	$16
#endasm
}
#endif

/****************************************************************************/

#ifdef L_bios_kbhit
kbhit()
{
#asm
  mov	ah,#1
  int	$16
  jz	nokey
  cmp	ax,#0
  jnz	dort
  mov	ax,#3
dort:
  ret
nokey:
  xor	ax,ax
#endasm
}
#endif

/****************************************************************************/

#ifdef L_bios_cputs
cputs(str)
   char * str;
{
      while(*str) putch(*str++);
}
#endif

/****************************************************************************/

#ifdef L_bios_getche
getche()
{
   static char linebuf[80];
   static int nextc = 0, endc=0;
   int rv;

   if (nextc >= endc)
   {
      endc = bios_readline(linebuf, sizeof(linebuf));
      nextc= 0;
   }
   if (endc <= nextc) return 3;
   rv = linebuf[endc++];
   if (endc == nextc) return '\r';
   return rv;
}
#endif

/****************************************************************************/

#ifdef L_bios_gotoxy
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

/****************************************************************************/

#ifdef L_bios_rdline
bios_rdline(buf, len)
char * buf;
int len;
{
   int ch;
   int pos=0;

   if( len < 0 ) { errno = EINVAL; return -1; }
   if( len == 0 )
   {
      if( kbhit() == 0 ) return 0;
      errno = EINTR;
      return -1;
   }
   if( len == 1 )
   {
      buf[0]=((ch=getch())&0xFF?ch&0xFF:((ch>>8)&0xFF|0x80));
      return 1;
   }

   for(ch=0;;)
   {
      if(ch != '\003')
      {
         ch = getch();
	 if( pos == 0 && (ch&0xFF) == 0 )
	 {
	    buf[0] = ((ch>>8)|0x80);
	    return 1;
	 }
	 ch &= 0x7F;
      }
      if( ch == '\r' )
      {
         putch('\n');
         buf[pos++] = '\n';
	 return pos;
      }
      if( ch >= ' ' && ch != 0x7F && pos < len-1)
         putch(buf[pos++] = ch);
      else if( (ch == '\003' || ch == '\b') && pos > 0 )
      {
         putch('\b'); putch(' '); putch('\b');
	 pos--;
      }
      else if( ch == '\003' )
         return 0;
      else
         putch('\007');
   }
}
#endif

/****************************************************************************/

#endif
#endif
#endif

