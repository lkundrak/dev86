/* Copyright (C) 1996 Robert de Bath <robert@mayday.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#ifdef __AS386_16__
#ifdef __STANDALONE__

#include <bios.h>

#ifdef L_bios_putc
bios_putc(c)
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
#endif

/****************************************************************************/

#ifdef L_bios_getc
bios_getc()
{
#asm
  xor	ax,ax
  int	$16
#endasm
}
#endif

/****************************************************************************/

#ifdef L_bios_khit
bios_khit()
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

#endif
#endif
