/*   simplified linux __ffillbuf.c
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
#include <unistd.h>
#include "stdio.h"

/* this function makes the mistake of assuming the buffer really DOES
need to be filled. */

/* RDB:
 *  Added simple processing for EOF and errors
 *  As this is _only_ called from getc() the memory footprint is smaller
 *  if it pretends to be getc in the complex case.
 */

int
__ffillbuf(stream)
FILE *stream;
{
  int stat;

  if( stream->fd < 0 || ferror(stream) ) return EOF;

  stat = read(stream->fd, (char *) stream->bufstart, stream->bufend - stream->bufstart);
  if( stat > 0 )
     stream->bufread = stream->bufstart + stat;
  else if( stat == 0 )
  {
     stream->fc_eof = 1;
     return EOF;
  }
  else
  {
     stream->fc_err = 1;
     return EOF;
  }

  stream->bufpos = stream->bufstart;

  return getc(stream);
}
