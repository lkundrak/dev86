
#include "monitor.h"

static unsigned memseg = 0, memlen = 0;

char buf[1];

/* If newseg == 0x0000 => Lowest address CS=$50
 * If newseg == 0x0001 => DS to 64k position
 * If newseg == 0x0002 => DS to 128k position
 * ...
 * If newseg == 0x0009 => DS to 576k position
 * If newseg == 0xFFFF => Highest address leaving Linux-i386 clear.
 *
 * All others are literal, will fail if would overlap with something important.
 */

void
relocator(newseg)
unsigned newseg;
{
#ifdef __STANDALONE__
   unsigned moved, codelen;
   unsigned es      = __get_es();

   /* If running under DOS don't relocate */
   if (__argr.x.cflag) return;

   /* Where do we start */
   if(memseg == 0)
   {
      extern int _heap_top;
      memseg = __get_cs();
      codelen = __get_ds()-memseg;
      __set_es(memseg-2);

      memlen = (((int)&_heap_top) >> 4);

      /*
      if (__deek_es(0) == 0x0301 ) memlen = (__deek_es(24) >> 4);
      */

      if( memlen == 0 ) memlen = 0x1000;
      memlen += codelen;
      __set_es(es);
   }

   if( newseg == 0 ) newseg = 0x50;
   if( newseg > 0 && newseg < 10 )
   {
      newseg = (newseg<<12) - (__get_ds() - __get_cs());
   }
   if( newseg < 0x50 ) return;

   if( newseg == 0xFFFF )
   {
      newseg = boot_mem_top;
      if( newseg > 0x9000 ) newseg = 0x9000;
      newseg -= memlen;
   }

   /* If the old area overlaps the new then fail */
   if( newseg >= memseg && newseg < memseg+memlen ) return;
   if( memseg >= newseg && memseg < newseg+memlen ) return;

   /* Copy segments, done in 32k chunks */
   for(moved=0; moved < memlen; )
   {
       unsigned int lump;
       if( memlen-moved <= 0x800 ) lump = memlen-moved; else lump = 0x800;

       __movedata(memseg+moved, 0, newseg+moved, 0, (lump<<4));
       moved += lump;
   }

   /* re-link int 0x80, this one is only an example (used by 'standalone.c') */
   /* __set_es(0); __doke_es(0x80*4+2, newseg); __set_es(es); */

   /* The actual jump ... */
   memseg = newseg;

#asm
   mov	ax,ds
   mov	bx,cs
   sub	ax,bx
   mov	bx,[_memseg]
   add	ax,bx
   push bx
   call	L_x
   mov	ds,ax
   mov	ss,ax
   mov	[_memseg],bx
#endasm
}
#asm
L_x:
   retf
#endasm

#else
}
#endif
