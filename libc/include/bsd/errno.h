/* make sure EWOULDBLOCK doesn't screw us up */
#ifndef _BSD_ERRNO_H
#define _BSD_ERRNO_H

#include_next <errno.h>
#undef EWOULDBLOCK
#define EWOULDBLOCK	EAGAIN

#endif /* _BSD_ERRNO_H */
