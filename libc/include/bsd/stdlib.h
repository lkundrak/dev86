/* some things shouldn't be macros, get out your barf bag */

#ifndef _BSD_STDLIB_H
#define _BSD_STDLIB_H

#undef __MALLOC_0_RETURNS_NULL
#define __MALLOC_0_RETURNS_NULL
#include_next <stdlib.h>
#undef atoi
#undef atol

#endif /* _BSD_STDLIB_H */
