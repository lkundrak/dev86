/* align.h - memory alignment requirements for bcc */

/* Copyright (C) 1992 Bruce Evans */

#ifndef S_ALIGNMENT
# define align(x) (x)
#else
# ifdef UNPORTABLE_ALIGNMENT
typedef unsigned pointerint_t;
#  define align(x) (((pointerint_t) (x) + (S_ALIGNMENT-1)) & ~(S_ALIGNMENT-1))
# else
#  define align(x) ((char *) (x) + (- (int) (x) & (S_ALIGNMENT-1)))
# endif
#endif

extern uoffset_t alignmask;	/* general alignment mask */
