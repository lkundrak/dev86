
#include "monitor.h"

int   x86 = 0;			/* CPU major number */
char *x86_name = "";		/* and it's name */
int   x86_emu = 0;		/* Is this a PC emulator ? */
int   x86_a20_closed = 1;	/* Is the A20 gate closed ? */
int   x86_fpu = 0;

int   x86_test = 0;		/* In test mode */

unsigned boot_mem_top = 0x2000;	/* Default 128k, the minimum */
long     main_mem_top = 0;	/* K of extended memory */

int a20_closed()
{
   register int v, rv = 0;
   if (x86_test) return 1;	/* If not standalone don't try */

   __set_es(0);
   v = __peek_es(512);
   __set_es(0xFFFF);
   if (v == __peek_es(512+16))
   {
      __set_es(0);
      __poke_es(512, v+1);
      __set_es(0xFFFF);
      if (v+1 == __peek_es(512+16))
	 rv = 1;
      __set_es(0);
      __poke_es(512, v);
   }
   return x86_a20_closed = rv;
}

static void asm_open_a20()
{
#asm
  call	empty_8042
  mov	al,#0xD1	! command write
  out	#0x64,al
  call	empty_8042
  mov	al,#0xDF	! A20 on
  out	#0x60,al
empty_8042:
  .word	0x00eb,0x00eb
  in	al,#0x64	! 8042 status port
  test	al,#2		! is input buffer full?
  jnz	empty_8042	! yes - loop, with no timeout!
#endasm
}

void open_a20() { if(!x86_test) asm_open_a20(); }

/* This calls the BIOS to open the A20 gate, officially this is only supported
   on PS/2s but if the normal routine fails we may as well try this.
 */
void asm_bios_open_a20()
{
#asm
  mov	ax,#$2401
  int	$15
  jc	bios_failed_a20
  xor	ax,ax
bios_failed_a20:
  mov	al,ah
  xor	ah,ah
#endasm
}

void bios_open_a20() { if(!x86_test) asm_bios_open_a20(); }

void cpu_check()
{
   static char *name_808x[] =
   {
      "8088", "8086", "80C88", "80C86", "NEC V20", "NEC V30", "808x Clone"
   };

   static char *name_8018x[] =
   {
      "80188", "80186", "8018x Clone"
   };
   static char cpubuf[] = "80x86+";
   int   c, major;

   c = cputype(0);
   x86_fpu = (c < 0);
   major = ((c >> 8) & 0x1F);
   c &= 0xFF;
   x86 = (major&0xF);
   if (major == 0)
   {
      if (c > 6) c = 6;
      x86_name = name_808x[c];
   }
   else if (major == 1)
   {
      if (c > 3) c = 3;
      x86_name = name_8018x[c];
   }
   else
   {
      cpubuf[2] = (major&0xF)+'0';
      cpubuf[5] = (major > 15 ? '+' : '\0');
      x86_name = cpubuf;
      if (c & 0x01) x86_emu = 1;	/* Already in protected mode !!! */
   }

#ifdef __STANDALONE__
   x86_test = x86_emu;
   if (__argr.x.cflag)
      x86_test = 1;
#else
   x86_test = 1;
#endif
}

void mem_check()
{
#ifndef __STANDALONE__
   main_mem_top = 16384;
   return;	/* If not standalone don't try */
#else
   {
#asm
  int	0x12		! Amount of boot memory
  mov	cl,#6
  sal	ax,cl		! In segments
  mov	[_boot_mem_top],ax

			! Next check for extended 
  mov	al,[_x86] 	! If we ain't got a 286+ we can't access it anyway
  cmp	al,#2
  jl  	is_xt

  mov	ah,#0x88	!
  int	0x15
  jnc	got_ext		! Error!? This should _not_ happen ... but ...
is_xt:
  xor	ax,ax
got_ext:
  mov	word ptr [_main_mem_top+2],#0
  mov	[_main_mem_top],ax

#endasm
  }

  if( main_mem_top == 0xFFFFL )
  {
     /* It say 64Mb-1k - Hmmmm I think it might be 128! */
  }
#endif
}

#define RIGHTS (0x93000000L)

static struct {
   char gdt0[8];
   char gdt1[8];
   unsigned short src_len;
   long           src_seg;
   unsigned short spad;
   unsigned short dst_len;
   long           dst_seg;
   unsigned short dpad;
   char gdt4[8];
   char gdt5[8];
} GDT = {
  "","",
  0xFFFF, RIGHTS, 0,
  0xFFFF, RIGHTS, 0,
  ""
};

ext_put(from, to, length)
unsigned int from, to, length;
{
   if(x86_test) return 3;
   GDT.src_seg = RIGHTS + from + ((long)__get_ds()<<4);
   GDT.dst_seg = RIGHTS + ((long)to<<8);
   if( length == 0xFFFF ) length = 0x8000;
   else                   length = ((length+1)>>1);
   return asm_copy(length);
}

ext_get(from, to, length)
unsigned int from, to, length;
{
   if(x86_test) return 3;
   GDT.src_seg = RIGHTS + ((long)from<<8);
   GDT.dst_seg = RIGHTS + to + ((long)__get_ds()<<4);
   if( length == 0xFFFF ) length = 0x8000;
   else                   length = ((length+1)>>1);
   return asm_copy(length);
}

static asm_copy(length)
{
#asm
#if !__FIRST_ARG_IN_AX__
  mov	bx,sp
  mov	ax,[bx+2]
#endif
  push	es
  push	si
  mov	cx,ax
  mov	ah,#$87
  push	ds
  pop	es
  mov	si,#_GDT
  int	$15
  jc	err
  xor	ax,ax
err:
  mov	al,ah
  xor	ah,ah
  pop	si
  pop	es
#endasm
}
