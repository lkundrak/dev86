/* align.h - memory alignment requirements for bcc */

/* Copyright (C) 1992 Bruce Evans */

#ifndef S_ALIGNMENT
# define align(x) (x)
#else
# if defined(__STDC__) && defined(_POSIX_SOURCE)
#  define align(x) (((ssize_t) (x) + (S_ALIGNMENT-1)) & ~(S_ALIGNMENT-1))
# else
#  define align(x) ((char *) (x) + (- (char) (x) & (S_ALIGNMENT-1)))
# endif
#endif

extern uoffset_T alignmask;	/* general alignment mask */
