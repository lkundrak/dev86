/*   fprintf.c for limited Linux libc
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
/* Thanks Alan for writing the hard routine for me :-)
 * Alan said that this works "most of the time".  Something tells me I'm making
 * it even worse! */
/* The basic idea here is to make fprintf the core routine.  printf obviously
   can just call fprintf with stdout followed by all of its arguments.
   sprintf() works using the fake file &the_sprintf.  It's marked as fully
   buffered, so that it will only write(2) when &the_sprintf->bufpos ==
   &the_sprintf->bufend, which I doubt will happen since &the_sprintf->bufend
   = 0.  The trick is that sprintf must set &the_sprintf->bufstart =
   &the_sprintf->bufpos = its first argument.  Not as orthagonal (is that
   the right word?) as glibc's facilities for non-files, but this isn't a
   library for people who have unlimited RAM budgets.  (not like the libc
   I use on linux/i586 enjoys an unlimited RAM budget either; I only have
   8 MB

   I'm not sure what the "correct" way to pass the variable arguments
   from one function to the next is.  Rather than pass the arguments
   themselves, I'm passing a pointer to them.  However, the following
   explaination from Alan is probably a polite way of saying it will not
   work on a 386 anyway.
   Joel Weber

   [ I've migrated all of this code over to the ELKS stdarg that I wrote.
     The accepted way to do it is for the wrapper function to accept a
     variable number of arguments, use stdarg to make an argument pointer,
     and then pass the argument pointer on to the core function, as I've
     done here.  This should definitely work on a 386, as the arguments
     are still passed in the stack, and stack order is maintained. -Nat ]
 */

/*
 *	This is NOT stunningly portable, but works
 *	for pretty dumb non ANSI compilers and is
 *	tight.	Adjust the sizes to taste.
 *
 *	Illegal format strings will break it. Only the
 *	following simple subset is supported
 *
 *	%x	-	hex
 *	%d	-	decimal
 *	%s	-	string
 *	%c	-	char
 *
 *	And the h/l length specifiers for %d/%x
 *
 *		Alan Cox.
 */

#include <stdarg.h>
#include "stdio.h"

/* 17 to make sure that there's room for the trailing newline.
   I'm not really sure if this is ideal... */
static char nstring[17]="0123456789ABCDEF";

static unsigned char *
__numout(long i, int base)
{
  static unsigned char out[16];
  int n;
  int flg = 0;
  unsigned long val;

  if (i<0 && base==10)
    {
      flg = 1;
      i = -i;
    }
  val = i;

  for (n = 0; n < 15; n++)
    out[n] = ' ';
  out[15] = '\0';
  n = 14;
  do{
    out[n] = nstring[val % base];
    n--;
    val /= base;
  }
  while(val);
  if(flg) out[n--] = '-';
  return &out[n+1];
}

static int
internal_fprintf(FILE * stream, __const char * fmt, va_list ap)
{
  register int c;
  int count = 0;
  int type, base;

  while(c=*fmt++)
    {
      if(c!='%')
	{
	  putc(c, stream);
	  count++;
	}
      else
	{
	  type=1;
	  do { c=*fmt++; } while( c=='.' || (c>='0' && c<='9'));
	  if( c == 0 ) break;
	  if(c=='h')
	    {
	      c=*fmt++;
	      type = 0;
	    }
	  else if(c=='l')
	    {
	      c=*fmt++;
	      type = 2;
	    }

	  switch(c)
	    {
	    case 'x':
	    case 'o':
	    case 'd':
	      if (c=='x') base=16;
	      if (c=='o') base=8;
	      if (c=='d') base=10;
	      {
		long val=0;
		switch(type)
		  {
		  case 0:
		    val=va_arg(ap, short);
		    break; 
		  case 1:
		    val=va_arg(ap, int);
		    break;
		  case 2:
		    val=va_arg(ap, long);
		    break;
		  }
		fputs((__const char *)__numout(val,base),stream);
	      }
	      break;
	    case 's':
	      {
		char *cp;
		cp=va_arg(ap, char *);
		while(*cp)
		    putc(*cp++, stream);
		break;
	      }
	    case 'c':
	      putc(va_arg(ap, int), stream);
	      break;
	    default:
	      putc(c, stream);
	    }
	}
    }
  return count;
}


int
fprintf(FILE * stream, __const char * fmt, ...)
{
  va_list ap;
  int retval;
  va_start(ap, fmt);
  retval=internal_fprintf(stream, fmt, ap);
  va_end(ap);
  return(retval);
}

int
printf(__const char * fmt, ...)
{
  va_list ap;
  int retval;
  va_start(ap, fmt);
  retval=internal_fprintf(stdout, fmt, ap);
  va_end(ap);
  return(retval);
}

/* This is a strange way of doing sprintf, but it should work */
int sprintf(char * s, __const char * fmt, ...)
{
  static FILE the_sprintf = {
    -1,
    0,
    0,
    0,
    0,
    _IOFBF,
    _MODE_WRITE,
    0, 0,
    0, 0};
  va_list ap;
  int retval;

  va_start(ap, fmt);
  the_sprintf.bufstart = the_sprintf.bufpos = (unsigned char *) s;
  the_sprintf.fc_err = 0;

  retval = internal_fprintf(&the_sprintf, fmt, ap);
  /* null-terminate the string */
  putc('\0', &the_sprintf);

  va_end(ap);
  return retval;
}





