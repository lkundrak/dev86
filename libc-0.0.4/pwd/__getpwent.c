/*
 * __getpwent.c - This file is part of the libc-8086/pwd package for ELKS,
 * Copyright (C) 1995, 1996 Nat Friedman <ndf@linux.mit.edu>.
 * 
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>

#define __PWD_LINE_BUFFER_SIZE 256

struct passwd *
__getpwent(int passwd_fd)
{
  static char pwd_line[__PWD_LINE_BUFFER_SIZE];
  unsigned short curr_index;

  /* field_num is the current field number, 0 through 6 */
  unsigned short field_num;
  
  /* field_length is the length of the current field */
  unsigned short field_length;

  /* field_begin is a pointer to the first character of the current field */
  char * field_begin;

  /* endptr is used to error-check the call to strtol() */
  char * endptr;

  /* the only static passwd structure.  we return a pointer to it */
  static struct passwd passwd;

  /* Gross, but small... */
  goto readline_start;

readline_restart:
  /* Advance to the end of the line */
  while (pwd_line[curr_index]!='\n' &&
	 read(passwd_fd, &pwd_line[curr_index], 1)==1);
  
readline_start:
  /* Initialize ourselves at the beginning of the line */
  curr_index=field_num=field_length=0;
  field_begin=pwd_line;
  while (read(passwd_fd, &pwd_line[curr_index], 1)==1)
    {
      if (curr_index==0)
	  if (*pwd_line=='#' || *pwd_line==' ' || *pwd_line=='\n' ||
	      *pwd_line=='\t')
	    goto readline_restart;

      field_length++;
      if (pwd_line[curr_index]==':' || pwd_line[curr_index]=='\n')
	{
	  pwd_line[curr_index]='\0';
	  switch (field_num)
	    {
	    case 0: /* login */
	      passwd.pw_name=field_begin;
	      break;
	    case 1: /* passwd */
	      passwd.pw_passwd=field_begin;
	      break;
	    case 2: /* GID */
	    case 3: /* UID */
	      if (field_num==2)
		passwd.pw_uid=strtol(field_begin, &endptr, 10);
	      else
		passwd.pw_gid=strtol(field_begin, &endptr, 10);
	      if (endptr!=(pwd_line+curr_index)) /* there were invalid
						    characters */
		  goto readline_restart;

	      curr_index-=field_length;
	      break;
	    case 4: /* GECOS field */
	      passwd.pw_gecos=field_begin;
	      break;
	    case 5: /* home directory */
	      passwd.pw_dir=field_begin;
	      break;
	    case 6: /* shell */
	      passwd.pw_shell=field_begin;
	      return &passwd;
	      break;
	    default: /* too many fields -- invalid line */
	      goto readline_restart;
	    }
	  if (field_num!=2 && field_num!=3)
	    field_begin=pwd_line+curr_index+1;
	  field_num++;
	  field_length=0;
	}
      curr_index++;
      if (curr_index>=__PWD_LINE_BUFFER_SIZE)
	{
	  if (field_num<=3)
	    goto readline_start;

	  if (*passwd.pw_gecos)
	    {
	      if (field_num==4) /* gecos.. least important */
		{
		  /* erase this field */
		  curr_index-=field_length;
		  field_num++;
		  passwd.pw_gecos=field_begin-1;
		  /* read until the next delimiter */
		  while ((read(passwd_fd, &pwd_line[curr_index], 1)==1) &&
			 pwd_line[curr_index]!=':');
		  
		}
	      else if (field_num>4) /* move everything back to the gecos... */
		{
		  unsigned short shift;
		  if (field_num==5)
		    passwd.pw_dir=field_begin;

		  shift=passwd.pw_dir-passwd.pw_gecos;
		  memcpy(passwd.pw_gecos,
			 passwd.pw_dir, pwd_line+curr_index-passwd.pw_dir);
		  passwd.pw_gecos--;
		  passwd.pw_dir=passwd.pw_gecos;
		  field_begin-=shift;
		  passwd.pw_shell-=shift;
		  curr_index-=shift;
		}
	    }
	  else
	    goto readline_restart;
	}
    }
  return NULL;
}

