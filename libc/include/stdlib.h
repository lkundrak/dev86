/* stdlib.h  <ndf@linux.mit.edu> */
#include <features.h>
#include <sys/types.h>

#ifndef __STDLIB_H
#define __STDLIB_H

/* Don't overwrite user definitions of NULL */
#ifndef NULL
#define NULL ((void *) 0)
#endif

/* For program termination */
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

extern void * malloc __P ((size_t));
extern void * calloc __P ((size_t, size_t));
extern void free __P ((void *));
extern void * realloc __P ((void *, size_t));

extern int rand __P ((void));
extern void srand __P ((unsigned int seed));

extern long strtol __P ((const char * nptr, char ** endptr, int base));
extern unsigned long strtoul __P ((const char * nptr,
				   char ** endptr, int base));
extern double strtod __P ((const char * nptr, char ** endptr));

#endif /* __STDLIB_H */
