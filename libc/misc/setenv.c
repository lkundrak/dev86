/* Copyright (C) 1992, 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.  */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

extern char ** environ;

int
setenv(name, value, replace)
__const char *name;
__const char *value;
int replace;
{
  register char **ep;
  register size_t size;
  __const size_t namelen = strlen (name);
  __const size_t vallen = strlen (value);
  int result = 0;

  size = 0;
  for (ep = environ; *ep != NULL; ++ep)
    if (!memcmp (*ep, name, namelen) && (*ep)[namelen] == '=')
      break;
    else
      ++size;
  
  if (*ep == NULL)
    {
      static char **last_environ = NULL;
      char **new_environ = (char **) malloc((size + 2) * sizeof(char *));
      if (new_environ == NULL)
	{
	  result = -1;
	  goto do_return;
	}
      (void) memcpy((void*) new_environ, (void*) environ, size * sizeof(char *));

      new_environ[size] = malloc (namelen + 1 + vallen + 1);
      if (new_environ[size] == NULL)
	{
	  free (new_environ);
	  errno = ENOMEM;
	  result = -1;
	  goto do_return;
	}
      memcpy (new_environ[size], name, namelen);
      new_environ[size][namelen] = '=';
      memcpy (&new_environ[size][namelen + 1], value, vallen + 1);

      new_environ[size + 1] = NULL;

      if (last_environ != NULL)
	free ((void*) last_environ);
      last_environ = new_environ;
      environ = new_environ;
    }
  else if (replace)
    {
      size_t len = strlen (*ep);
      if (len < namelen + 1 + vallen)
	{
	  char *new = malloc (namelen + 1 + vallen + 1);
	  if (new == NULL)
	    {
	      result = -1;
	      goto do_return;
	    }
	  *ep = new;
	}
      memcpy (*ep, name, namelen);
      (*ep)[namelen] = '=';
      memcpy (&(*ep)[namelen + 1], value, vallen + 1);
    }

do_return:
  return result;
}

void
unsetenv(name)
__const char *name;
{
  register char **ep;
  register char **dp;
  __const size_t namelen = strlen (name);

  for (dp = ep = environ; *ep != NULL; ++ep)
    if (memcmp (*ep, name, namelen) || (*ep)[namelen] != '=')
      {
	*dp = *ep;
	++dp;
      }
  *dp = NULL;
}
