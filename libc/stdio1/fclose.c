/*   simplified linux fclose.c
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

int fclose (stream)
FILE *stream;
{
  /* only allow fclose() on what was fopen()ed. */
  /*
     This isn't right, there's nothing wrong with fclosing stdin
     but the next fopen/open will be given fd 0 and so become stdin
     Of course you do have to be a little careful :-)
     RDB
   */

  if ((stream == stdin) || (stream == stdout) || (stream == stderr)) return EOF;

  if (fflush(stream)) return EOF;

  if (close(stream->fd)) return EOF;

  free(stream->bufstart);
  free(stream);
  return 0;
}
