/*   fgets.c for limited linux stdio
     Copyright (C) 1996  Joel N. Weber II <nemo@koa.iolani.honolulu.hi.us>
     
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
   RDB BZZZT! This should only read upto and including any newline!
*/

char *fgets(s, count, f)
char *s;
size_t count;
FILE *f;
{
  char *ret;
  register size_t i;
  register int ch;

  ret = s;
  for(i=count; i>0; i--)
  {
     ch = getc(f);
     if( ch == EOF )
     {
        if(s==ret) return 0;
	break;
     }
     *s++ = (char) ch;
     if( ch == '\n' ) break;
  }
  *s = 0;

  if( ferror(f) ) return 0;
  return ret;
}
