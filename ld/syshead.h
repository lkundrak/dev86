
#ifndef POSIX_HEADERS_MISSING
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#ifndef STDC_HEADERS_MISSING
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#endif

#ifdef MSDOS
#undef POSIX_HEADERS_MISSING
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>

#undef min
#define R_OK	0
#define mode_t		unsigned short
#define SEEK_SET	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2
#define VERSION 	"MSDOS Compile"
#endif

/******************************************************************************/

/* EEEEyuk!! */

#ifdef __STDC__
#define P(x)	x
#else
#define P(x)	()
#endif

#ifdef STDC_HEADERS_MISSING
extern int errno;
char *strcat P((char *dest, const char *src));
unsigned long strtoul P((const char *s, char **endptr, int base));
void exit P((int status));
void *malloc P((unsigned size));
int strncmp P((const char *s1, const char *s2, unsigned n));
char *strncpy P((char *dest, const char *src, unsigned n));
char * strcpy P((char* dest, char* src));
void *memset P((void *s, int c, unsigned n));
#endif

#ifdef POSIX_HEADERS_MISSING

#define R_OK	0
int access P((const char *path, int amode));
#define SEEK_SET	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

#define mode_t		unsigned short
#define off_t		long

int chmod P((const char *path, int mode));
int close P((int fd));
int creat P((const char *path, int mode));
int fstat P((int fd, struct stat *statbuf));
off_t lseek P((int fd, off_t offset, int whence));
int open P((const char *path, int oflag, ...));
int read P((int fd, void *buf, unsigned nbytes));
mode_t umask P((int oldmask));
int write P((int fd, const void *buf, unsigned nbytes));
#endif

#ifndef O_RDONLY
#define O_RDONLY	0
#endif
#ifndef O_WRONLY
#define O_WRONLY	1
#endif
#ifndef O_RDWR
#define O_RDWR		2
#endif
