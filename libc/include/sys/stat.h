#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <linuxmt/stat.h>
#include <sys/types.h>
#include <features.h>

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

int stat __P((__const char * __path, struct stat * __statbuf));
int lstat __P((__const char * __path, struct stat * __statbuf));
int fstat __P((int __fd, struct stat * __statbuf));

#endif
