#ifndef __LINUXMT_TYPES_H
#define __LINUXMT_TYPES_H

#include "../arch/types.h"

/* Throw away _FUNCTION parameters - the syntax is ripped off of Minix's
   _PROTOTYPE.  Considering Borland did the same thing to MFC on a bigger
   scale, I don't think PH will mind :) */

/* Yes, this should be in arch/types.h too */

#define _FUNCTION(function, params) function()
#define _VFUNCTION(functiom, params) (*function) ()

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

typedef __u16 dev_t;
typedef __u16 ino_t;
typedef __u32 tcflag_t;
typedef __u8  cc_t;

#endif

