
#include "monitor.h"

#ifndef NOMONITOR
int   x86 = 3;			/* CPU major number */
char *x86_name = "";		/* and it's name */
int   x86_emu = 0;		/* Is this a PC emulator ? */
int   x86_fpu = 0;
#endif

int   x86_test = 0;		/* In test mode */

unsigned boot_mem_top = 0x2000;	/* Default 128k, the minimum */
long     main_mem_top = 0;	/* K of extended memory */

void cpu_check()
{
#ifndef NOMONITOR
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

   x86_test = x86_emu;
   if (x86<3)
      x86_test = 1;
#endif

#ifdef __STANDALONE__
   if (__argr.x.cflag)
      x86_test = 1;
#else
   x86_test = 1;
#endif
}

void mem_check()
{
   if (x86_test) {
      main_mem_top = 0;
      return;
   }

#asm
  int	0x12		! Amount of boot memory
  mov	cl,#6
  sal	ax,cl		! In segments
  mov	[_boot_mem_top],ax

  mov	ah,#0x88	! Next check for extended 
  int	0x15
  jnc	got_ext		! Error!
is_xt:
  xor	ax,ax
got_ext:
  mov	word ptr [_main_mem_top+2],#0
  mov	[_main_mem_top],ax
#endasm

   /* Try int $15 EAX=$E820 */
   {
      struct e820_dat {
	 unsigned long base_lo, base_hi;
	 unsigned long len_lo, len_hi;
	 long addr_type;
      } e820_item;
      long epoll = 0;

      do
      {
	 e820_item.addr_type = 0;
#asm
	 mov eax,#$E820
	 mov ebx,.mem_check.epoll[bp]
	 mov ecx,#20
	 mov edx,#$534D4150
	 push ds
	 pop es
	 lea di,.mem_check.e820_item[bp]
	 int $15
	 jnc got_e820
	 xor ebx,ebx
got_e820:
	 mov .mem_check.epoll[bp],ebx
#endasm
	 if (e820_item.addr_type == 1
	       && e820_item.base_hi == 0
	       && e820_item.base_lo == 0x100000L)
	 {
	    /* XXX Later ... */
	    if (e820_item.len_hi) main_mem_top = 0x40000;
	    else
	       main_mem_top = (e820_item.len_lo >> 10);
	    return;
	 }
      }
      while(epoll);
   }

   /* Try int $15 EAX=$E801 */
   {
      unsigned int mem_64, mem_16;	/* For int $15,AX=$E801 */
#asm
      mov ax,#$E801
      int $15
      jc  no_e801
      mov .mem_check.mem_16[bp],ax
      mov .mem_check.mem_64[bp],bx
#endasm
      main_mem_top = ((unsigned long)mem_64<<6) + mem_16;
#asm
no_e801:
#endasm
   }
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

mem_write(buffer, baseaddr, sectno, sectcount)
void * buffer;
long baseaddr;
unsigned  sectno, sectcount;
{
   if(x86_test) return 0;
   /* In an EMU we can't write to high mem but
      we'll pretend we can for debuggering */

   baseaddr += ((long)sectno<<9);
   if( baseaddr < 0xA0000L )
   {
      __movedata(__get_ds(), buffer, 
	         (unsigned)(baseaddr>>4), (unsigned)(baseaddr&0xF),
		 sectcount * 512);
      return 0;
   }

   GDT.src_seg = RIGHTS + (unsigned)buffer + ((long)__get_ds()<<4);
   GDT.dst_seg = RIGHTS + baseaddr;
   return asm_copy(sectcount << 8);
}

mem_read(buffer, baseaddr, sectno)
void * buffer;
long baseaddr;
int  sectno;
{
   if(x86_test) return 3;
   baseaddr += ((long)sectno<<9);

   GDT.dst_seg = RIGHTS + (unsigned)buffer + ((long)__get_ds()<<4);
   GDT.src_seg = RIGHTS + baseaddr;
   return asm_copy(256);
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
