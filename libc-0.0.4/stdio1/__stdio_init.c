/*   simplified linux __stdio_init.c
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
  OK, Complete hackup here, I've removed the need for the init function.
  Not also I've make the types into 'unsigned char' this is so it doesn't
  return EOF when it really means '\377'
 */

static unsigned char __stdinbuf[BUFSIZ], __stdoutbuf[BUFSIZ], __stderrbuf[80];

static FILE __the_stdin = {
   0,
   __stdinbuf,
   __stdinbuf,
   __stdinbuf,
   __stdinbuf+sizeof(__stdinbuf),
   _IOLBF,
   _MODE_READ,
   0, 0,
   0, 0, 1
};

static FILE __the_stdout = {
   1,
   __stdoutbuf,
   __stdoutbuf,
   __stdoutbuf,
   __stdoutbuf+sizeof(__stdoutbuf),
   _IOLBF,
   _MODE_WRITE,
   0, 0,
   0, 0, 1
};

static FILE __the_stderr = {
   2,
   __stderrbuf,
   __stderrbuf,
   __stderrbuf,
   __stderrbuf+sizeof(__stderrbuf),
   _IOLBF,
   _MODE_WRITE,
   0, 0,
   0, 0, 1
};

FILE *stdin        = &__the_stdin,
     *stdout       = &__the_stdout,
     *stderr       = &__the_stderr;
