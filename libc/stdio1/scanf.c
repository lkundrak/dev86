/*   scanf.c for limited Linux libc
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

/* This scanf code was derived from my printf code, which in turn was
 * derived from Alan Cox's printk.  It was tested by [no one so far...]
 */

/* Disclaimer: I haven't even THOUGHT about testing this.  If you do decide
 * to be brave and test it, and it doesn't work, you have the following
 * options:
 *	o	Send me mail telling me it doesn't work, which I will ignore
 *		since I already knew that.
 *	o	Fix it yourself
 *	o	isolate the problem and send it to the list, and maybe
 *		I'll fix it
 */

/* BTW, the above comment may be destroyed once this has been tested.  It's
 * not nessisary for my great great grandchildren to see it when they
 * take Ancient Linux History...
 */

/* Incidentally, if it doesn't work, it may well be because I never bothered
 * to learn how to use scanf()...I briefly tried it for the usaco qualification
 * round, but had some wierd problems...ask me after mid February...BTW the
 * problems had nothing to do with scanf(); it was still broken (hopelessly)
 * when I went to getchar().
 */

/* Note, too that the current version probably assumes some things with
 * variable argument handling that it shouldn't.  Don't expect this to
 * work on a 386 in 32 bit mode, don't expect this to work on a Z80,
 * or anything other than an 8086.  In fact, don't expect it to even
 * work on an 8086 ;-)
 */

/* One of these days the headers will work painlessly... */
/* #include <linuxmt/types.h> */
/* #include <linuxmt/fcntl.h> */
#include "stdio.h"

/* note that we assume base == 10... */
static int numin(int *num, int base, FILE *stream, int c)
{
	if ((c < '0') || (c > '9')) return 0;
	*num = 0;
	while ((c >= '0') && (c <= '9')) {
		*num = *num * base + c - '0';
		c = getc(stream);
	}
	ungetc(c, stream);
}

/* currently, the h/l specifications are ignored */
static int internal_scanf(stream,fmt,a1)
FILE *stream;
char *fmt;
int *a1;
{
	unsigned char *p=(char *)a1;
	char c;
	int inc;
	int count = 0;

	while(c=*fmt++)
	{
		do {
			inc = getc(stream);
		} while ((inc == ' ') || (inc == '\t') || (inc == '\n'));

		if ((c == '\n') || (c == '\t') || (c == ' '))
			;
		else if (c!='%') {
			if (c != inc) {
				ungetc(inc, stream);
				return count;
			}
		} else {
			int len=2;
			c=*fmt++;
			if(c=='h') c=*fmt++;
			else if(c=='l') c=*fmt++;
			
			switch(c)
			{
				case 'x':
					if (numin(p, 16, stream, inc)) count++;
					else return count;
					p+=2;
					break;
				case 'd':
					if (numin(p, 10, stream, inc)) count++;
					else return count;
					p+=2;
					break;
				case 's':
				{
					char *cp=*((char **)p);
					p+=sizeof(char *);
					while(*cp)
                                           	putc(cp++, stream);
					while ((inc != ' ') && (inc != '\t')
					     && (inc != '\n') && (inc != EOF)) {
						*cp++ = inc;
						inc = getc(stream);
					}
					ungetc(inc, stream);
					break;
				}
				case 'c':
					*p++ = inc;
					p++;
					break;
				default:
                                   	putc('?', stream);
			}
		}
	}
	return count;
}


int fscanf(stream,fmt,a1)
FILE *stream;
char *fmt;
int a1;
{
  return internal_fscanf(stream,fmt,&a1);
}

int scanf(fmt,a1)
char *fmt;
int a1;
{
  return internal_fscanf(stdout,fmt,&a1);
}
