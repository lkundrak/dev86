#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <linuxmt/stat.h>
#include <sys/types.h>
#include <features.h>

#ifdef __AS386_32__
/* This is for Linux-386, ho hum, I wish BCC could compile the proper one */
#define mknod	__dv32_mknod
#define stat	__dv32_stat
#define lstat	__dv32_lstat
#define fstat	__dv32_fstat

struct stat {
	dev_t		st_dev;
	unsigned short __pad1;
	ino_t		st_ino;
	umode_t		st_mode;
	nlink_t		st_nlink;
	uid_t		st_uid;
	gid_t		st_gid;
	dev_t		st_rdev;
	unsigned short __pad2;
	off_t		st_size;
	unsigned long	st_blksize;
	unsigned long	st_blocks;
	time_t		st_atime;
	unsigned long	__unused1;
	time_t		st_mtime;
	unsigned long	__unused2;
	time_t		st_ctime;
	unsigned long	__unused3;
	unsigned long	__unused4;
	unsigned long	__unused5;
};
#endif

#ifdef __AS386_16__
struct stat
{
	dev_t st_dev;
	ino_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	off_t st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
};
#endif

int stat __P((__const char * __path, struct stat * __statbuf));
int lstat __P((__const char * __path, struct stat * __statbuf));
int fstat __P((int __fd, struct stat * __statbuf));

/* hysterical raisins */
#define S_IREAD		S_IRUSR /* read permission, owner */
#define S_IWRITE	S_IWUSR /* write permission, owner */
#define S_IEXEC		S_IXUSR /* execute/search permission, owner */
#endif
