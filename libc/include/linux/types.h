#ifndef __LINUX_TYPES_H
#define __LINUX_TYPES_H

#include <asm/types.h>

typedef __u32 off_t;
typedef __u16 pid_t;
typedef __u16 uid_t;
typedef __u16 gid_t;
typedef __u32 time_t;
typedef __u16 umode_t;
typedef __u16 nlink_t;
typedef __u16 mode_t;
typedef __u32 loff_t;
typedef __u32 speed_t;

typedef __u32 dev_t;
typedef __u32 ino_t;
typedef __u32 tcflag_t;
typedef __u8  cc_t;
typedef __u32 size_t;

typedef int   ptrdiff_t;

#endif

