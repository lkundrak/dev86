/* Copyright (C) 1995-1997 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <errno.h>

/****************************************************************************/

#if defined(__GNUC__) && defined(__i386__)
extern char * __brk_addr;
extern char * __brk();

#ifdef L___brk_addr
char * __brk_addr = 0;	/* This holds the current return for sbrk(0) */

         /* "mov	%ebx,4(%esp),%ebx\n\t" "mov	$45,%eax\n\t" */

char * 
__brk(char * val)
{
   char * retval;
   __asm__(
         "int	$0x80"
	 : "=a" (retval)
	 : "0" (45), "b" (val)
   );
   return retval;
}

__brk_addr_init()
{
   if( __brk_addr == 0 && (__brk_addr = __brk(0)) == 0 )
   {
      errno = ENOMEM;
      return -1;
   }
   return 0;
}
#endif

#ifdef L_sbrk
char *
sbrk(brk_off)
int brk_off;
{
   char * new_brk;
   if( __brk_addr_init() ) return (char*)-1;
   if( brk_off == 0 ) return __brk_addr;

   new_brk = __brk_addr + brk_off;
   __brk_addr = __brk(new_brk);
   if( __brk_addr != new_brk )
   {
      errno = ENOMEM;
      return (char*)-1;
   }
   return __brk_addr - brk_off;
}
#endif

#ifdef L_brk
int
brk(new_brk)
char * new_brk;
{
   if( __brk_addr_init() ) return -1;

   __brk_addr = __brk(new_brk);
   if( __brk_addr == new_brk ) return 0;
   errno = ENOMEM;
   return -1;
}
#endif

#endif
