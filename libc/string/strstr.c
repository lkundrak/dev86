/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>

#if 1
/* We've now got a nice fast strchr and memcmp use them */

char *
strstr(s1, s2)
char *s1; char *s2;
{
   register int l = strlen(s2);
   register char * p = s1;

   if( l==0 ) return p;

   while (p = strchr(p, *s2))
   {
      if( memcmp(p, s2, l) == 0 )
         return p;
      p++;
   }
   return (char *) 0;
}

#else
/* This is a nice simple self contained strstr,
   now go and work out why the GNU one is faster :-) */

char *strstr(str1, str2)
char *str1, *str2;
{
    register char *Sptr, *Tptr;
    int	len = strlen(str1) -strlen(str2) + 1;

    if (*str2)
	for (; len > 0;	len--, str1++){
	    if (*str1 != *str2)
		continue;

	    for	(Sptr =	str1, Tptr = str2; *Tptr != '\0'; Sptr++, Tptr++)
		if (*Sptr != *Tptr)
		    break;

	    if (*Tptr == '\0')
		return (char*) str1;
	}

    return (char*)0;
}
#endif
