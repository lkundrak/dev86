
#include <i86_funcs.h>

static unsigned memseg = 0, memlen = 0;

char buf[1];

void
relocator(newseg)
unsigned newseg;
{
   unsigned moved, codelen;
   unsigned es      = __get_es();

   if( x86_test ) return;	/* I don't think so! */

   /* Where do we start */
   if(memseg == 0)
   {
      memseg = __get_cs();
      codelen = __get_ds()-memseg;
      __set_es(memseg-2);
      memlen = __deek_es( 24 );
      memlen >>=4;
      if( memlen == 0 ) memlen = 0x1000;
      memlen += codelen;
      __set_es(es);
   }

   if( newseg < 0x50 ) return;

   if( newseg == 0xFFFF )
   {
      newseg = boot_mem_top;
      if( newseg > 0x90000 ) newseg = 0x90000;
      newseg -= memlen;
   }

   /* If the old area overlaps the new then fail */
   if( newseg >= memseg && newseg < memseg+memlen ) return;
   if( memseg >= newseg && memseg < newseg+memlen ) return;

   /* Copy segments, done in 32k chunks */
   for(moved=0; moved < memlen; )
   {
       unsigned int lump;
       if( memlen <= 0x800 ) lump = memlen; else lump = 0x800;

       __movedata(memseg+moved, 0, newseg+moved, 0, (lump<<4));
       moved += lump;
   }

   /* re-link int 0x80
   __set_es(0);
   __doke_es(0x80*4+2, newseg);
   __set_es(es);
   */

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
