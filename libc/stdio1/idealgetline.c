/*   idealgetline.c -- my idea of an ideal getline function for stdio
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

/* I actually intend to use this in 32 bit programs, unlike the other stuff
   in this lib.  IMHO GNU getline is broken.  It is too hard to use.

   Just pass this function a FILE * and it will retrieve a line for you using
   getc().  It will realloc() exactly the amount of memory needed, and will
   generate error messages for non-ascii characters on stderr.

   This may not be your ideal.  It probably generates far too many errors.
   It doesn't work well for those who don't use English (but since
   Linux-less-than-32 libc and kernels are each being produced by one
   person in the US and one in the UK, this isn't a problem).  It probably
   is not what you (or I) want for interactive input.

   You're welcome to modify this routine to meet your needs.  However, if
   you change the semantics significantly, please change the name.

   (Maybe I should have put my own name on it so there will be no confusion
   about who thought it was ideal). */

#include <stdio.h>
#include <malloc.h>

char *idealgetline(f)
FILE *f;
{
  char c;
  char *ret;
  int size = 0, bufsize = 256;

  ret = malloc(256);
  c = getc(f);
  while ((c != EOF) && (c != '\n')){
    if ((c >= ' ') && (c <= 126)){
      ret[size] = c;
      size++;
      if (size == bufsize){
        bufsize += 256;
        ret = realloc(ret, bufsize);
      }
    } else {
      fprintf(stderr, "Unexpected character 0x%x encountered in input", c);
      free(ret);
      return 0;
    }
  }
  ret[size] = '\0';
  size++;
  return realloc(ret, size);
}
