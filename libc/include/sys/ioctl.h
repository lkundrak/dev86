
#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H
#include <features.h>
#include <linuxmt/ioctl.h>

extern int ioctl __P((int __fildes, int __cmd, ...));

#endif
