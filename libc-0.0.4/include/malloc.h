
#ifndef __MALLOC_H
#define __MALLOC_H
#include <features.h>

/*
 * Mini malloc allows you to use a less efficient but smaller malloc the
 * cost is about 100 bytes of code in free but malloc (700bytes) doesn't
 * have to be linked. Unfortunatly memory can only be reused if everything
 * above it has been freed
 * 
 */

void free __P((void *));
void *malloc __P((unsigned int));
void *realloc __P((void *, unsigned int));
void *alloca __P((unsigned int));

extern void *(*__alloca_alloc) __P((unsigned int));

#ifdef __LIBC__
#define __MINI_MALLOC__
#endif

#ifdef __MINI_MALLOC__
#define malloc(x) ((*__alloca_alloc)(x))
#endif

#endif
