#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <features.h>
#include <sys/types.h>
#include __SYSINC__(stat.h)

int stat __P((__const char * __path, struct stat * __statbuf));
int lstat __P((__const char * __path, struct stat * __statbuf));
int fstat __P((int __fd, struct stat * __statbuf));

/* hysterical raisins */
#define S_IREAD		S_IRUSR /* read permission, owner */
#define S_IWRITE	S_IWUSR /* write permission, owner */
#define S_IEXEC		S_IXUSR /* execute/search permission, owner */
#endif
