#ifndef __MSDOS_FCNTL_H
#define __MSDOS_FCNTL_H

/* Definitions taken from Ralf Brown interrupt list and Linux kernel */

#define O_ACCMODE	0x03
#define O_RDONLY	0x00
#define O_WRONLY	0x01
#define O_RDWR		0x02

#define O_DENYMODE	0x70
#define O_DENYALL	0x10
#define O_DENYWRITE	0x20
#define O_DENYREAD	0x30
#define O_DENYNONE	0x40
#define O_SETFD		0x80	/* Close on exec flag for open */

#define O_CREAT		0x100	/* Moved on DOS to avoid DENY's */
#define O_TRUNC		0x200
#define O_APPEND	0x400
#define O_EXCL		0x800

#define O_BINARY	0x0000
#define O_TEXT	 	0x8000	/* This right ? Who cares! */

#define F_DUPFD		0	/* dup */
#define F_GETFD		1	/* get f_flags */
#define F_SETFD		2	/* set f_flags */
#define F_GETFL		3	/* more flags (cloexec) */
#define F_SETFL		4
#define F_GETLK		5
#define F_SETLK		6
#define F_SETLKW	7

#endif
