/* align.h - memory alignment requirements for linker */

/* Copyright (C) 1994 Bruce Evans */

#ifndef S_ALIGNMENT
# define align(x)
#else
# define align(x) ((x) = ((int) (x) + (S_ALIGNMENT-1)) & ~(S_ALIGNMENT-1))
				/* assumes sizeof(int) == sizeof(char *) */
#endif
