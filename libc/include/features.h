
#ifndef __FEATURES_H
#define __FEATURES_H

#ifdef __STDC__

#define __P(x) x
#define __const const

/* Almost ansi */
#if __STDC__ != 1
#define const
#define volatile
#endif

#else /* K&R */

#define __P(x) ()
#define __const
#define const
#define volatile

#endif

/* No C++ */
#define __BEGIN_DECLS
#define __END_DECLS

/* GNUish things */
#define __CONSTVALUE
#define __CONSTVALUE2

#include <sys/cdefs.h>

#endif

