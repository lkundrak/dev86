/* align.h - memory alignment requirements for linker */

/* Copyright (C) 1994 Bruce Evans */

#ifndef S_ALIGNMENT
# define align(x)
#else
# define align(x) ((x)=(void*) 	\
		   ((char *)(x) + ((S_ALIGNMENT-(int)(x)) & (S_ALIGNMENT-1))))
#endif






/* * assumes sizeof(int) == sizeof(char *) *
# define align(x) ((x) = (void *)(((int) (x) + (S_ALIGNMENT-1)) & ~(S_ALIGNMENT-1)))
*/

