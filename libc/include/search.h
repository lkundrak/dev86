/* Copyright (C) 1993 Ulrich Drepper

This file is intended to be included in the GNU C Library and the
Linux C Library. So the copyright notice will be:


Copyright (C) 1993 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For now the file can be distributed under the LGPL.  */

#ifndef _SEARCH_H
#define _SEARCH_H

#include <features.h>

#define __need_size_t
#define __need_NULL
#include <stddef.h>

__BEGIN_DECLS

#ifndef __COMPAR_FN_T
#define __COMPAR_FN_T
typedef int (*__compar_fn_t) __P ((__const __ptr_t, __const __ptr_t));
#endif

/* for use with hsearch(3) */

typedef struct entry { char *key; char *data; } ENTRY;
typedef enum { FIND, ENTER } ACTION;

extern ENTRY * hsearch __P((ENTRY __item, ACTION __action));
extern int     hcreate __P((unsigned __nel));
extern void    hdestroy __P((void));


/* The tsearch routines are very interesting. They make many
 * assumptions about the compiler. It assumpts that the first field
 * in node must be the "key" field, which points to the datum.
 * Everything depends on that. It is a very tricky stuff. H.J.
 */
/* For tsearch */
typedef enum { preorder, postorder, endorder, leaf } VISIT;

extern void *tsearch __P((__const void * __key, void **__rootp,
		__compar_fn_t compar));

extern void *tfind __P((__const void * __key, __const void ** __rootp,
		__compar_fn_t compar));

extern void *tdelete __P((__const void * __key, void ** __rootp,
		__compar_fn_t compar));

#ifndef __ACTION_FN_T
#define __ACTION_FN_T
/* FYI, the first argument should be a pointer to "key".
 * Please read the man page for details.
 */
typedef void (*__action_fn_t) __P((__const void *__nodep,
				   __const VISIT __value,
				   __const int __level));
#endif

extern void twalk __P((__const void * __root, __action_fn_t action));


extern void * lfind __P((__const void * __key, __const void * __base,
                         size_t * __nmemb, size_t __size,
                         __compar_fn_t __compar));

extern void * lsearch __P((__const void * __key, __const void * __base,
                         size_t * __nmemb, size_t __size,
                         __compar_fn_t __compar));

__END_DECLS

#endif /* search.h */
