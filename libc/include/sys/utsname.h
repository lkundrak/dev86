#ifndef __SYS_UTSNAME_H
#define __SYS_UTSNAME_H

#include <features.h>
#include <sys/param.h>

struct utsname {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
};

extern int uname __P ((struct utsname * __utsbuf));

#endif
