
#include <bios.h>
#include <errno.h>
#asm
entry _int_80		! Tell ld86 we really do need this file.
			! then call the init stuff before main.

  loc	1		! Make sure the pointer is in the correct segment
auto_func:		! Label for bcc -M to work.
  .word	_pre_main	! Pointer to the autorun function
  .word no_op		! Space filler cause segs are padded to 4 bytes.
  .text			! So the function after is also in the correct seg.
#endasm

void int_80();

static void pre_main()
{
   /* Set the int 0x80 pointer to here */
   __set_es(0);
   __doke_es(0x80*4+0,   int_80);
   __doke_es(0x80*4+2, __get_cs());
   bios_coninit();
}

void int_80()
{
#asm
SYS_EXIT=1
SYS_FORK=2
SYS_READ=3
SYS_WRITE=4
SYS_OPEN=5
SYS_CLOSE=6
SYS_CHDIR=12
SYS_LSEEK=19
ENOSYS=38

  push	es
  push	si
  push	di
  push	dx
  push	cx
  push	bx
  cmp	ax,#SYS_READ
  jne	L1
  call	_func_read
  jmp	L0
L1:
  cmp	ax,#SYS_WRITE
  jne	L2
  call	_func_write
  jmp	L0
L2:
  cmp	ax,#SYS_LSEEK
  jne	L3
  call	_func_lseek
  jmp	L0
L3:
  cmp	ax,#SYS_EXIT
  jne	L4
  call	_func_exit
  jmp	L0
L4:
  mov	ax,#-ENOSYS
L0:
  pop	bx
  pop	cx
  pop	dx
  pop	di
  pop	si
  pop	es
  iret
#endasm
}

func_lseek() { return -38; }

func_write(bx,cx,dx,di,si,es)
int bx,dx;
char * cx;
{
   register int v, c;
   if(bx == 1 || bx == 2)
   {
      for(v=dx; v>0; v--)
      {
         c= *cx++;
         if( c == '\n') bios_putc('\r');
	 bios_putc(c);
      }
      return dx;
   }
   return -EBADF;
}

func_read(bx,cx,dx,di,si,es)
int bx,dx;
char * cx;
{
   if(bx == 0) return read_line(cx, dx);
   return -EBADF;
}

read_line(buf, len)
char * buf;
int len;
{
   int ch;
   int pos=0;

   if( len == 1 )
   {
      buf[0]=((ch=bios_getc())&0xFF?ch&0xFF:((ch>>8)&0xFF|0x80));
      return 1;
   }

   for(ch=0;;)
   {
      if(ch != '\003')
      {
         ch = bios_getc();
	 if( pos == 0 && (ch&0xFF) == 0 )
	 {
	    buf[0] = ((ch>>8)|0x80);
	    return 1;
	 }
	 ch &= 0x7F;
      }
      if( ch == '\r' )
      {
         bios_putc('\r'); bios_putc('\n');
         buf[pos++] = '\n';
	 return pos;
      }
      if( ch >= ' ' && ch != 0x7F && pos < len-1)
         bios_putc(buf[pos++] = ch);
      else if( (ch == '\003' || ch == '\b') && pos > 0 )
      {
         bios_putc('\b'); bios_putc(' '); bios_putc('\b');
	 pos--;
      }
      else if( ch == '\003' )
         return 0;
      else
         bios_putc('\007');
   }
}

#define CTRL(x) ((x)&0x1F)
static int last_attr = 0x07;
static int con_mode;
static int con_size = 0x184F;
static int con_colour = 0;

bios_coninit()
{
#asm
  mov	ax,#$0F00
  int	$10
  mov	_con_mode,ax
#endasm
  if( (con_mode &0xFF) > 39 ) con_size = (con_size&0xFF00) + (con_mode&0xFF);
  if( (con_mode&0xFF00) != 0x700)
     con_colour = 1;
}

bios_putc(c)
int c;
{
static char tbuf[3];
static int  tcnt=0;
   if(tcnt)
   {
      tbuf[tcnt++] = c;
      if( tcnt < 3 && (tbuf[0] != CTRL(']') || tbuf[1] < '`' || tbuf[1] > 'p'))
         return;
      if( tbuf[0] == CTRL('P') )
      {
         if( tbuf[1] >= 32 && tbuf[1] <= 56 
          && tbuf[2] >= 32 && tbuf[2] <= 111 )
            asm_cpos((tbuf[1]-32), (tbuf[2]-32));
      }
      else
      {
         if( tbuf[1] >= '`' )
            last_attr = ( (tbuf[1]&0xF) | (last_attr&0xF0));
         else
            last_attr = ( (tbuf[2]&0xF) | ((tbuf[1]&0xF)<<4));

         if( !con_colour )
	    last_attr = (last_attr&0x88) + ((last_attr&7)?0x07:0x70);
      }
      tcnt=0;
      return;
   }
   if( c & 0xE0 ) { asm_colour(last_attr) ; asm_putc(c); }
   else switch(c)
   {
   case CTRL('L'):
      asm_cpos(0,0);
      asm_cls();
      break;
   case CTRL('P'):
   case CTRL(']'):
      tbuf[tcnt++] = c;
      break;
   default:
      asm_putc(c);
      break;
   }
   return;
}

static asm_putc(c)
{
#asm
#if !__FIRST_ARG_IN_AX__
  mov	bx,sp
  mov	ax,[bx+2]
#endif
  mov	ah,#$0E
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

static asm_cls()
{
#asm
  push	bp	! Bug in some old BIOS's
  !mov	ax,#$0500
  !int	$10
  mov	ax,#$0600
  mov	bh,_last_attr
  mov	cx,#$0000
  mov	dx,_con_size
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

bios_getc()
{
#asm
  xor	ax,ax
  int	$16
#endasm
}

static void be_safe()
{
#asm
   iret
#endasm
}

func_exit(bx,cx,dx,di,si,es)	/* AKA reboot! */
{
   __set_es(0);
   __doke_es(0xE6*4+2,__get_cs());
   __doke_es(0xE6*4+0,be_safe);
#asm
  mov	ax,#$FFFF
  int	$E6		! Try to exit DOSEMU
  mov	ax,#$0040	! If we get here we're not in dosemu.
  mov	es,ax
  seg es
  mov	[$72],#$1234	! Warm reboot.
  jmpi	$0000,$FFFF
#endasm
}
