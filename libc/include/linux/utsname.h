#ifndef __LINUX_UTSNAME_H
#define __LINUX_UTSNAME_H

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
