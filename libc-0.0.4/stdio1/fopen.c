/*   simplified linux fopen.c
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
#include <malloc.h>

FILE * fopen(name, openmode)
char *name;
char *openmode;
{
  FILE *new;
  int openplus=0;
  char basemode;

  basemode = openmode[0];
  while (openmode[0] != 0){
    switch(openmode[0]){
      case '+': openplus=1; break;
    }
    openmode++;
  }

  new = malloc(sizeof(new));
  if( new == 0 ) return 0;
  new->bufread = new->bufpos = new->bufstart = malloc(BUFSIZ);
  if( new->bufstart == 0 ) { free(new) ; return 0; }

  new->bufend = new->bufstart + BUFSIZ;
  new->buffer_mode = _IOFBF;
  new->iotrans = 0;
  new->fd = -1;
  if (openplus){
    new->file_mode = _MODE_RDWR;
    switch (basemode){
      case 'a':
        new->fd = open(name, O_RDWR | O_APPEND | O_CREAT);
        break;
      case 'r':
        new->fd = open(name, O_RDWR);
        break;
      case 'w':
        new->fd = open(name, O_RDWR | O_TRUNC | O_CREAT);
        break;
    }
  } else switch (basemode){
    case 'a':
      new->fd = open(name, O_WRONLY | O_APPEND | O_CREAT);
      new->file_mode = _MODE_WRITE;
      break;
    case 'r':
      new->fd = open(name, O_RDONLY);
      new->file_mode = _MODE_READ;
      break;
    case 'w':
      new->fd = open(name, O_WRONLY | O_TRUNC | O_CREAT);
      new->file_mode = _MODE_WRITE;
      break;
  }
  if( new->fd < 0 ) 
  {
     free(new->bufstart);
     free(new);
     return 0;
  }
  return new;
}
