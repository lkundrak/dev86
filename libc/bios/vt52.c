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

extern void (*__smart_putch)();

#define CTRL(x) ((x)&0x1F)
static int last_attr = 0x07;
static int con_mode;
static unsigned char con_height = 24, con_width = 79;

static int con_colour = 0;
static unsigned char con_row, con_col;

vt52_putch(c)
int c;
{
static int ctrl = 0;
   int new_attr;
   __smart_putch = vt52_putch();

   if( con_mode==0 ) asm_coninit();

   switch( ctrl )
   {
   case 1:
      ctrl=0;
      switch(c)
      {
      case 'A': if( con_row ) con_row--; asm_cpos(con_row, con_col); break;
      case 'B': if( con_row != con_height ) con_row++;
                asm_cpos(con_row, con_col); break;
      case 'C': if( con_col != con_height ) con_col++;
                asm_cpos(con_row, con_col); break;
      case 'D': if( con_col ) con_col--; asm_cpos(con_row, con_col); break;
      case 'E': last_attr = 0x07; asm_cls();
      case 'H': asm_cpos(0,0); break;
      case 'J': asm_cls(); break;
      case 'K': break;
      case 'R': ctrl = 2; break; 	/* Foreground */
      case 'S': ctrl = 3; break; 	/* Background */
      case 'Y': ctrl = 4; break;	/* ttypos */
      }
      break;
   case 2: ctrl=0; new_attr = (last_attr & 0xF0) + (c&0xF);
	   if(0) {
   case 3: ctrl=0; new_attr = (last_attr & 0x0F) + (c<<4); 
           }
	   switch(c)
	   {
	   case '_': if( !con_colour ) last_attr = (last_attr&0x88) + 1;
	             break;
	   case '!': last_attr  = (last_attr&0x88) + 0x70; break;
	   case ' ': last_attr  = 0x07; break;
	   case '+': last_attr |= 0x08; break;
	   case '*': last_attr |= 0x80; break;

	   default: if( con_colour )
	               last_attr = new_attr;
	   }
           break;
   case 4: ctrl=5; con_col = c-' '; break;
   case 5: ctrl=0; con_row = c-' '; asm_cpos(con_row, con_col); break;
      break;

   default:
      if( c & 0xE0 )
         { asm_colour(last_attr) ; asm_putc(c); }
      else switch(c)
      {
      default:
	 asm_putc(c);
	 break;
      case CTRL('I'):
	 asm_gpos();
	 con_col = ((con_col+8)& -8);
	 asm_cpos(con_row, con_col);
	 break;
      case CTRL('L'):
	 asm_cpos(0,0);
	 asm_cls();
	 break;
      case CTRL('['):
         ctrl = 1;
	 asm_gpos();
	 break;
      }
      break;
   }
}

static asm_coninit()
{
#asm
  mov	ax,#$0F00
  int	$10
  mov	_con_mode,ax
#endasm
  if( (con_mode &0xFF) > 39 ) con_width = (con_mode>>8);
  if( (con_mode&0xFF) != 0x7)
     con_colour = 1;
}

static asm_putc(c)
{
#asm
#if !__FIRST_ARG_IN_AX__
  mov	bx,sp
  mov	ax,[bx+2]
#endif
  cmp	al,#$0A
  jne	not_nl
  mov	ax,#$0E0D
  mov	bx,#7
  int	$10
  mov	al,#$0A
not_nl:
  mov	ah,#$0E
  mov	bx,#7
  int	$10
#endasm
}

static asm_cls()
{
#asm
  push	bp	! Bug in some old BIOS`s
  !mov	ax,#$0500
  !int	$10
  mov	ax,#$0600
  mov	bh,_last_attr
  mov	cx,#$0000
  mov	dl,_con_width
  mov	dh,_con_height
  int	$10
  pop	bp
#endasm
}

static asm_cpos(r,c)
{
#asm
#if __FIRST_ARG_IN_AX__
  mov	bx,sp
  mov	dh,al
  mov	ax,[bx+2]
  mov	dl,al
#else
  mov	bx,sp
  mov	ax,[bx+2]
  mov	dh,al
  mov	ax,[bx+4]
  mov	dl,al
#endif
  mov	ah,#$02
  mov	bx,#7
  int	$10
#endasm
}

static asm_colour(c)
{
#asm
#if __FIRST_ARG_IN_AX__
  mov	bx,ax
#else
  mov	bx,sp
  mov	bx,[bx+2]
#endif
  mov	ah,#$08
  int	$10
  mov	ah,#$09
  mov	cx,#1
  int	$10
#endasm
}

static asm_gpos()
{
#asm
  mov	ah,#$03
  mov	bx,#7
  int	$10
  mov	[_con_row],dh
  mov	[_con_col],dl
  mov	ax,cx
#endasm
}

#endif
#endif
#endif

