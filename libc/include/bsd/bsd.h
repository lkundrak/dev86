/* bsd.h - simplify porting BSD programs to Linux - rick sladkey */

/* make sure BSD features are enabled, i.e. __USE_BSD and _FAVOR_BSD */

/* cpp in gcc 2.3.3 is broken.
#ifndef _BSD_BSD_H
#define _BSD_BSD_H
*/

#define _BSD_SOURCE 1
#include <features.h>
#include <endian.h>

/* some BSD progs expect MIN and MAX to be defined */

#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))

/* make sure we get L_SET and L_INCR, which is in a different place */

#include <sys/file.h>

/* BSD has slight non-POSIX names (and meanings :-) for some things */

#define FAPPEND		O_APPEND 

#include <limits.h>

/* (absolute) max # characters in exec arglist */

#define NCARGS		ARG_MAX

/* ftpd uses this as bits per byte, I don't know why it's called NBBY */

#define NBBY		CHAR_BIT

/* gloss over slight differences between BSD direct and POSIX dirent */
  
#define d_namlen	d_reclen

#if 0

/* See <bsd/signal.h> */

typedef void	sig_t;

#endif

/* #endif _BSD_BSD_H */
