/* Copyright (C) 1996,2004 Robert de Bath <robert@debath.thenet.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */
#include <string.h>

#include "error_list.h"

char *
strerror(err)
int err;
{
   static char retbuf[20];

   if( err > 0 && err <= sys_nerr )
      return sys_errlist[err];

   strcpy(retbuf, "Error ");
   strcpy(retbuf+6, itoa(err));
   return retbuf;
}
