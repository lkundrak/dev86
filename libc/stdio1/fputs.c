/*   simplified linux fputs.c
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
#include <string.h>
#include "stdio.h"

int
fputs(__const char * string, FILE * stream)
{
  if (stream->buffer_mode != _IONBF){
    while (string[0] != 0){
      putc(string[0], stream);
      string++;
    }
  } else write(stream->fd, string, strlen(string));
  return 0;
}
