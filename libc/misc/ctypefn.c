/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/*
 *  CTYPE.C	Character classification and conversion
 */

#include <ctype.h>

#undef	toupper
#undef	tolower

int toupper(c)
int c;
{
   return(islower(c) ? (c ^ 0x20) : (c));
}

int tolower(c)
int c;
{
   return(isupper(c) ? (c ^ 0x20) : (c));
}
