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

#ifdef ANSI_CON
#define MAX_ARGS 8
static int colconv[] = { 0, 4, 2, 6, 1, 5, 3, 7 };
static int vidcnt = 0;
static int ansi_2 = 0;
static int ansi_argc = 0;
static int ansi_argv[MAX_ARGS];
static void do_ansi();

ansi_putch(c)
int c;
{
   if( con_mode==0 ) asm_coninit();
   __smart_putch = ansi_putch;

   if( vidcnt == 0 || c < ' ' || c > '~' )
   {
      switch(c & 0xFF)
      {
      default:
         asm_colour(last_attr);
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
         vidcnt=1;
	 break;
      }
      return;
   }

   /* ... ansi/vt100 interpretation, this collects _all_ ansi control strings */
   switch(vidcnt)
   {
   case 1: for(ansi_argc=0; ansi_argc<MAX_ARGS; ansi_argc++)
	      ansi_argv[ansi_argc] = 0;
	   ansi_2 = 0;
	   ansi_argc = 0;
	   vidcnt++;

           if( c != '[' )
           {
	      do_ansi(c + 0x1B00, ansi_argc, ansi_argv);
	      vidcnt = 0;
	   }
           break;

   case 2: if( c == ';' )
           {
	      if( ansi_argc < MAX_ARGS )
	         ansi_argc++;
	   }
	   else if( c >= '0' && c <= '9' )
	      ansi_argv[ansi_argc] = ansi_argv[ansi_argc] * 10 + c - '0';
	   else if( c >= '@' )
	   {
	      do_ansi(c+ansi_2, ++ansi_argc, ansi_argv);
	      vidcnt = 0;
	   }
	   else
	      ansi_2 = (c<<8);
           break;

   default: vidcnt = 0; /* This definitly won't happen ... definitly ... */
   }
}

void 
do_ansi(ctrl, argc, argv)
int ctrl, argc, *argv;
{
   switch(ctrl)
   {
   case 'A':
   case 'B':
   case 'C':
   case 'D':
      asm_gpos();
      if( argv[0] < 1 ) argv[0] = 1;
      switch(ctrl)
      {
      case 'A': if( argv[0]>con_row ) con_row=0; else con_row-=argv[0]; break;
      case 'B': if( argv[0]+con_row>con_height )
                   con_row = con_height;
                else
                   con_row += argv[0];
                break;
      case 'C': if( argv[0]+con_col>con_width )
                   con_col = con_width;
                else
                   con_col += argv[0];
                break;
      case 'D': if( argv[0]>con_col ) con_col=0; else con_col-=argv[0]; break;
      }
      asm_cpos(con_row, con_col);
      break;

   case 'H':
             if( --argv[0] < 0 ) argv[0] = 0; 
             if( --argv[1] < 0 ) argv[1] = 0; 
	     asm_cpos(argv[0],argv[1]);
	     break;
   case 'J': if( argv[0] == 2 ) asm_cls();
             break;
   case 'm':
      {
         int ar;
	 for(ar=0; ar<argc; ar++) switch(argv[ar])
	 {
	 case 0: last_attr  = 0x07; break;
	 case 1: last_attr |= 0x08; break;
	 case 5: last_attr |= 0x80; break;
	 case 7: last_attr  = 0x70; break;
         case 30: case 31: case 32: case 33:
         case 34: case 35: case 36: case 37:
	    if( con_colour )
     	       last_attr = ((last_attr&0xF8)|(colconv[argv[ar]-30]));
     	    break;
         case 40: case 41: case 42: case 43:
         case 44: case 45: case 46: case 47:
	    if( con_colour )
     	       last_attr = ((last_attr&0x8F)|((colconv[argv[ar]-40])<<4));
     	    break;
         default: break;
	 }
      }
   }
}
#endif

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
