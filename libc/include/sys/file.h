#ifndef _SYS_FILE_H
#define _SYS_FILE_H

#include <features.h>
#include <fcntl.h>

#ifndef L_SET

#define L_SET           0       /* absolute offset */
#define L_INCR          1       /* relative to current offset */
#define L_XTND          2       /* relative to end of file */

#endif

#ifndef LOCK_SH

/* Operations for the `flock' call.  */
#define LOCK_SH       1    /* Shared lock.  */
#define LOCK_EX       2    /* Exclusive lock.  */
#define LOCK_UN       8    /* Unlock.  */

/* Can be OR'd in to one of the above.  */
#define LOCK_NB       4    /* Don't block when locking.  */

#endif

__BEGIN_DECLS

/* Apply or remove an advisory lock, according to OPERATION,
   on the file FD refers to.  */
extern int flock __P ((int __fd, int __operation));

__END_DECLS

#endif
