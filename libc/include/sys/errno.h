#ifndef __ERRNO_H
#define __ERRNO_H

#include <features.h>
#include __SYSINC__(errno.h)

#ifdef  __USE_BSD
extern int sys_nerr;
extern char *sys_errlist[];
#endif
#ifdef  __USE_GNU
extern int _sys_nerr;
extern char *_sys_errlist[];
#endif

extern int	errno;

__BEGIN_DECLS

extern void	perror __P ((__const char* __s));
extern char*	strerror __P ((int __errno));

__END_DECLS

#endif
