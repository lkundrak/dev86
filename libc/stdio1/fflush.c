/*   simplified linux fflush.c
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

int fflush(stream)
FILE *stream;
{
  if (stream == 0) return 0;
  if ((stream->file_mode == _MODE_WRITE)
    ||(stream->file_mode == (_MODE_WRITE & _MODE_RDWR))){
      write(stream->fd, (char *) stream->bufstart,
	    stream->bufpos - stream->bufstart);
      stream->bufpos = stream->bufstart;
    }
  return 0;
}
/* TODO: return EOF or 0; support NULL stream */
/* The only place an error can come from is the write; you're not checking RDB
 */
