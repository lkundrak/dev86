/*
 * __getgrent.c - This file is part of the libc-8086/grp package for ELKS,
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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <grp.h>
#include "config-grp.h"

/*
 * This is the core group-file read function.  It behaves exactly like
 * getgrent() except that it is passed a file descriptor, and  getgrent()
 * is just a wrapper for this function.
 */
struct group *
__getgrent(int grp_fd)
{
#ifndef GR_SCALE_DYNAMIC  
  static char line_buff[GR_MAX_LINE_LEN];
  static char * members[GR_MAX_MEMBERS];
#else
  static char * line_buff = NULL;
  static char ** members = NULL;
#endif
  static struct group group;
  register char * ptr;
  char * field_begin;
  char * endptr;
  short member_num;
  short line_index;


  /* We use the restart label to handle malformatted lines */
restart:
  line_index=0;

#ifndef GR_SCALE_DYNAMIC
  *line_buff='\0';
  /* Read the line into the static buffer */
  while (1)
    {
      if (read(grp_fd, &line_buff[line_index], 1)<=0)
	return NULL;
      if (*line_buff=='#' || *line_buff==' ' || *line_buff=='\n' ||
	  *line_buff=='\t')
	{
	  while (line_buff[line_index]!='\n' &&
		 read(grp_fd, &line_buff[line_index], 1)==1);
	  goto restart;
	}
      if (line_buff[line_index]=='\n')
	{
	  line_buff[line_index]='\0';
	  break;
	}
      line_index++;
      if (line_index==GR_MAX_LINE_LEN)
	{
	  while (line_buff[line_index]!='\n' &&
		 read(grp_fd, &line_buff[line_index], 1)==1);
	  return NULL;
	}
    }
      
#else /* !GR_SCALE_DYNAMIC */
  line_buff=realloc(line_buff, 1);
  while (1)
    {
      if (read(grp_fd, &line_buff[line_index], 1)<=0)
	  return NULL;
      if (*line_buff=='#' || *line_buff==' ' || *line_buff=='\n' ||
	  *line_buff=='\t')
	{
	  while (line_buff[line_index]!='\n' &&
		 read(grp_fd, &line_buff[line_index], 1)==1);
	  goto restart;
	}
      if (line_buff[line_index]=='\n')
	{
	  line_buff[line_index]='\0';
	  break;
	}
      line_index++;
      line_buff=realloc(line_buff, line_index+1);
    }
#endif /* GR_SCALE_DYNAMIC */

  /* Now parse the line */
  group.gr_name=line_buff;
  ptr=strchr(line_buff, ':');
  if (ptr==NULL) goto restart;
  *ptr++='\0';

  group.gr_passwd=ptr;
  ptr=strchr(ptr, ':');
  if (ptr==NULL) goto restart;
  *ptr++='\0';

  field_begin=ptr;
  ptr=strchr(ptr, ':');
  if (ptr==NULL) goto restart;
  *ptr++='\0';

  group.gr_gid=(gid_t) strtoul(field_begin, &endptr, 10);
  if (*endptr!='\0') goto restart;

  member_num=0;
  field_begin=ptr;

#ifndef GR_SCALE_DYNAMIC
  while ((ptr=strchr(ptr, ','))!=NULL)
    {
      *ptr='\0';
      ptr++;
      members[member_num]=field_begin;
      field_begin=ptr;
      member_num++;
    }
  if (*field_begin=='\0')
    members[member_num]=NULL;
  else
    {
      members[member_num]=field_begin;
      members[member_num+1]=NULL;
    }
#else /* !GR_SCALE_DYNAMIC */
  if (members!=NULL)
    free (members);
  members=(char **) malloc(1*sizeof(char *));
  while ((ptr=strchr(ptr, ','))!=NULL)
    {
      *ptr='\0';
      ptr++;
      members[member_num]=field_begin;
      field_begin=ptr;
      member_num++;
      members=(char **)realloc((void *)members, (member_num+1)*sizeof(char *));
    }
  if (*field_begin=='\0')
      members[member_num]=NULL;
  else
    {
      members[member_num]=field_begin;
      members[member_num+1]=NULL;
    }
#endif /* GR_SCALE_DYNAMIC */

  group.gr_mem=members;
  return &group;
}
