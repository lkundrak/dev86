/*   simplified linux fputc.c
     Copyright (C) 1995  Joel N. Weber II

     This program is free software; you can redistribute it and/or
     modify it under the terms of the GNU General Public License
     as published by the Free Software Foundation; either version 2
     of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "stdio.h"

/*
 * Make this not dependent on putc() then we can:
       #define putc(c, fp) fputc(c, fp)
 * if memory is _really_ tight.
 */

#undef putc
#define putc(c, stream)							\
 (((stream)->bufpos[0] = (c)),						\
  ((stream)->bufpos++),							\
  ((((stream)->bufpos == (stream)->bufend)				\
    ||((stream)->buffer_mode == _IONBF)					\
    ||(((stream)->buffer_mode == _IOLBF)				\
      && ((stream)->bufpos != (stream)->bufstart)			\
      && ((stream)->bufpos[-1] == '\n')))				\
        ? fflush(stream):0))

#define new_putc(c, stream)	\
 ((unsigned char)(							\
    ((stream)->bufpos>=(stream)->bufread) ? fputc((c), (stream))	\
                                          : *(stream)->bufpos++ = (c)	\
 ))

int
fputc(int c, FILE * stream)
{
#ifdef __MSDOS__
   if( c == '\n' && stream->iotrans ) fputc('\r', stream);
#endif
   return putc(c, stream);
}



