/*	ls 3.2 - List files.				Author: Kees J. Bot
 *
 * About the amount of bytes for heap + stack under Minix:
 * Ls needs a average amount of 42 bytes per unserviced directory entry, so
 * scanning 10 directory levels deep in an ls -R with 100 entries per directory
 * takes 42000 bytes of heap.  So giving ls 10000 bytes is tight, 20000 is
 * usually enough, 40000 is pessimistic.
 */

/* Compile with the proper -D flag for your system:
 *
 *	_MINIX		Minix (1.5 or later)
 *	BSD		BSD derived (has st_blocks)
 *	AMOEBA		Amoeba's emulation of UNIX
 */

/* The array _ifmt[] is used in an 'ls -l' to map the type of a file to a
 * letter.  This is done so that ls can list any future file or device type
 * other than symlinks, without recompilation.  (Yes it's dirty.)
 */
char _ifmt[] = "0pcCd?bB-?l?s???";

#define ifmt(mode)	_ifmt[((mode) >> 12) & 0xF]

#define nil 0
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef AMOEBA
#undef S_IFLNK	/* Liars */
#endif
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <fcntl.h>
#if BSD || __minix_vmd
#include <termios.h>
#endif
#if __minix_vmd
#include <sys/ioctl.h>
#endif

#ifndef major
#define major(dev)	((int) (((dev) >> 8) & 0xFF))
#define minor(dev)	((int) (((dev) >> 0) & 0xFF))
#endif

#if !_MINIX
#define SUPER_ID	uid	/* Let -A flag be default for SUPER_ID == 0. */
#else
#define SUPER_ID	gid
#endif

#ifdef S_IFLNK
int (*status)(const char *file, struct stat *stp);
#else
#define status	stat
#endif

/* Basic disk block size is 512 except for one niche O.S. */
#if _MINIX
#define BLOCK	1024
#else
#define BLOCK	 512
#endif

/* Some terminals ignore more than 80 characters on a line.  Dumb ones wrap
 * when the cursor hits the side.  Nice terminals don't wrap until they have
 * to print the 81st character.  Wether we like it or not, no column 80.
 */
#ifdef TIOCGWINSZ
int ncols= 79;
#else
#define ncols	79
#endif

#define NSEP	2	/* # spaces between columns. */

#ifdef TIOCGWINSZ
#define MAXCOLS	150
#else
#define MAXCOLS	(1 + (ncols / (1+NSEP)))	/* Max # of files per line. */
#endif

char *arg0;	/* Last component of argv[0]. */
int uid, gid;	/* callers id. */
int ex= 0;	/* Exit status to be. */
int istty;	/* Output is on a terminal. */

/* Safer versions of malloc and realloc: */

void heaperr(void)
{
	fprintf(stderr, "%s: Out of memory\n", arg0);
	exit(-1);
}

void *allocate(size_t n)
/* Deliver or die. */
{
	void *a;

	if ((a= malloc(n)) == nil) heaperr();
	return a;
}

#define reallocate	rllct	/* Same as realloc under some compilers. */

void *reallocate(void *a, size_t n)
{
	if ((a= realloc(a, n)) == nil) heaperr();
	return a;
}

char allowed[] = "acdfgilnqrstu1ACFLMRTX";
char flags[sizeof(allowed)];

char arg0flag[] = "cfmrtx";	/* These in argv[0] go to upper case. */

void setflags(char *flgs)
{
	int c;

	while ((c= *flgs++) != 0) {
		if (strchr(allowed, c) == nil) {
			fprintf(stderr, "Usage: %s -[%s] [file ...]\n",
				arg0, allowed);
			exit(1);
		} else
		if (strchr(flags, c) == nil)
			flags[strlen(flags)] = c;
	}
}

int present(int f)
{
	return f == 0 || strchr(flags, f) != nil;
}

void report(char *f)
/* Like perror(3), but in the style: "ls: junk: No such file or directory. */
{
	fprintf(stderr, "%s: %s: %s\n", arg0, f, strerror(errno));
	ex= 1;
}

/* Two functions, uidname and gidname, translate id's to readable names.
 * All names are remembered to avoid searching the password file.
 */
#define NNAMES	(1 << (sizeof(int) + sizeof(char *)))
enum whatmap { PASSWD, GROUP };

struct idname {		/* Hash list of names. */
	struct idname	*next;
	char		*name;
	uid_t		id;
} *uids[NNAMES], *gids[NNAMES];

char *idname(unsigned id, enum whatmap map)
/* Return name for a given user/group id. */
{
	struct idname *i;
	struct idname **ids= &(map == PASSWD ? uids : gids)[id % NNAMES];

	while ((i= *ids) != nil && id < i->id) ids= &i->next;

	if (i == nil || id != i->id) {
		/* Not found, go look in the password or group map. */
		char *name= nil;
		char noname[3 * sizeof(uid_t)];

		if (!present('n')) {
			if (map == PASSWD) {
				struct passwd *pw= getpwuid(id);

				if (pw != nil) name= pw->pw_name;
			} else {
				struct group *gr= getgrgid(id);

				if (gr != nil) name= gr->gr_name;
			}
		}
		if (name == nil) {
			/* Can't find it, weird.  Use numerical "name." */
			sprintf(noname, "%u", id);
			name= noname;
		}

		/* Add a new id-to-name cell. */
		i= allocate(sizeof(*i));
		i->id= id;
		i->name= allocate(strlen(name) + 1);
		strcpy(i->name, name);
		i->next= *ids;
		*ids= i;
	}
	return i->name;
}

#define uidname(uid)	idname((uid), PASSWD)
#define gidname(gid)	idname((gid), GROUP)

/* Path name construction, addpath adds a component, delpath removes it.
 * The string path is used throughout the program as the file under examination.
 */

char *path;	/* Path name constructed in path[]. */
int plen= 0, pidx= 0;	/* Lenght/index for path[]. */

void addpath(int *didx, char *name)
/* Add a component to path. (name may also be a full path at the first call)
 * The index where the current path ends is stored in *pdi.
 */
{
	if (plen == 0) path= (char *) allocate((plen= 32) * sizeof(path[0]));

	if (pidx == 1 && path[0] == '.') pidx= 0;	/* Remove "." */

	*didx= pidx;	/* Record point to go back to for delpath. */

	if (pidx > 0 && path[pidx-1] != '/') path[pidx++]= '/';

	do {
		if (*name != '/' || pidx == 0 || path[pidx-1] != '/') {
			if (pidx == plen)
				path= (char *) reallocate((void *) path,
						(plen*= 2) * sizeof(path[0]));
			path[pidx++]= *name;
		}
	} while (*name++ != 0);

	--pidx;		/* Put pidx back at the null.  The path[pidx++]= '/'
			 * statement will overwrite it at the next call.
			 */
}

#define delpath(didx)	(path[pidx= didx]= 0)	/* Remove component. */

int field = 0;	/* (used to be) Fields that must be printed. */
		/* (now) Effects triggered by certain flags. */

#define F_INODE		0x001	/* -i */
#define F_BLOCKS	0x002	/* -s */
#define F_EXTRA		0x004	/* -X */
#define F_MODE		0x008	/* -lMX */
#define F_LONG		0x010	/* -l */
#define F_GROUP		0x020	/* -g */
#define F_BYTIME	0x040	/* -tuc */
#define F_ATIME		0x080	/* -u */
#define F_CTIME		0x100	/* -c */
#define F_MARK		0x200	/* -F */
#define F_TYPE		0x400	/* -T */
#define F_DIR		0x800	/* -d */

struct file {		/* A file plus stat(2) information. */
	struct file	*next;	/* Lists are made of them. */
	char		*name;	/* Null terminated name. */
	ino_t		ino;
	mode_t		mode;
	uid_t		uid;
	gid_t		gid;
	nlink_t		nlink;
	dev_t		rdev;
	off_t		size;
	time_t		mtime;
	time_t		atime;
	time_t		ctime;
#if BSD
	long		blocks;
#endif
};

void setstat(struct file *f, struct stat *stp)
{
	f->ino=		stp->st_ino;
	f->mode=	stp->st_mode;
	f->nlink=	stp->st_nlink;
	f->uid=		stp->st_uid;
	f->gid=		stp->st_gid;
	f->rdev=	stp->st_rdev;
	f->size=	stp->st_size;
	f->mtime=	stp->st_mtime;
	f->atime=	stp->st_atime;
	f->ctime=	stp->st_ctime;
#if BSD
	f->blocks=	stp->st_blocks;
#endif
}

#define	PAST	(26*7*24*3600L)	/* Half a year ago. */
/* Between PAST and FUTURE from now a time is printed, otherwise a year. */
#define FUTURE	(15*60L)	/* Fifteen minutes. */

static char *timestamp(struct file *f)
/* Transform the right time field into something readable. */
{
	struct tm *tm;
	time_t t;
	static time_t now;
	static int drift= 0;
	static char date[] = "Jan 19  2038";
	static char month[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

	t= f->mtime;
	if (field & F_ATIME) t= f->atime;
	if (field & F_CTIME) t= f->ctime;

	tm= localtime(&t);
	if (--drift < 0) { time(&now); drift= 50; }	/* limit time() calls */

	if (t < now - PAST || t > now + FUTURE) {
		sprintf(date, "%.3s %2d  %4d",
			month + 3*tm->tm_mon,
			tm->tm_mday,
			1900 + tm->tm_year);
	} else {
		sprintf(date, "%.3s %2d %02d:%02d",
			month + 3*tm->tm_mon,
			tm->tm_mday,
			tm->tm_hour, tm->tm_min);
	}
	return date;
}

char *permissions(struct file *f)
/* Compute long or short rwx bits. */
{
	static char rwx[] = "drwxr-x--x";

	rwx[0] = ifmt(f->mode);
	/* Note that rwx[0] is a guess for the more alien file types.  It is
	 * correct for BSD4.3 and derived systems.  I just don't know how
	 * "standardized" these numbers are.
	 */

	if (field & F_EXTRA) {		/* Short style */
		int mode = f->mode, ucase= 0;

		if (uid == f->uid)	/* What group of bits to use. */
			/* mode<<= 0, */
			ucase= (mode<<3) | (mode<<6);
			/* Remember if group or others have permissions. */
		else
		if (gid == f->gid)
			mode<<= 3;
		else
			mode<<= 6;

		rwx[1]= mode&S_IRUSR ? (ucase&S_IRUSR ? 'R' : 'r') : '-';
		rwx[2]= mode&S_IWUSR ? (ucase&S_IWUSR ? 'W' : 'w') : '-';

		if (mode&S_IXUSR) {
			static char sbit[]= { 'x', 'g', 'u', 's' };

			rwx[3]= sbit[(f->mode&(S_ISUID|S_ISGID))>>10];
			if (ucase&S_IXUSR) rwx[3] += 'A'-'a';
		} else
			rwx[3]= f->mode&(S_ISUID|S_ISGID) ? '=' : '-';
		rwx[4]= 0;
	} else {		/* Long form. */
		char *p= rwx+1;
		int mode= f->mode;

		do {
			p[0] = (mode & S_IRUSR) ? 'r' : '-';
			p[1] = (mode & S_IWUSR) ? 'w' : '-';
			p[2] = (mode & S_IXUSR) ? 'x' : '-';
			mode<<= 3;
		} while ((p+=3) <= rwx+7);

		if (f->mode&S_ISUID) rwx[3]= f->mode&(S_IXUSR>>0) ? 's' : '=';
		if (f->mode&S_ISGID) rwx[6]= f->mode&(S_IXUSR>>3) ? 's' : '=';
		if (f->mode&S_ISVTX) rwx[9]= f->mode&(S_IXUSR>>6) ? 't' : '=';
	}
	return rwx;
}

void numeral(int i, char **pp)
{
	char itoa[3*sizeof(int)], *a=itoa;

	do *a++ = i%10 + '0'; while ((i/=10) > 0);

	do *(*pp)++ = *--a; while (a>itoa);
}

#define K	1024L		/* A kilobyte counts in multiples of K */
#define T	1000L		/* A megabyte in T*K, a gigabyte in T*T*K */

char *cxsize(struct file *f)
/* Try and fail to turn a 32 bit size into 4 readable characters. */
{
	static char siz[] = "1.2m";
	char *p= siz;
	off_t z;

	siz[1]= siz[2]= siz[3]= 0;

	if (f->size <= 5*K) {	/* <= 5K prints as is. */
		numeral((int) f->size, &p);
		return siz;
	}
	z= (f->size + K-1) / K;

	if (z <= 999) {		/* Print as 123k. */
		numeral((int) z, &p);
		*p = 'k';	/* Can't use 'K', looks bad */
	} else
	if (z*10 <= 99*T) {	/* 1.2m (Try ls -X /dev/at0) */
		z= (z*10 + T-1) / T;	/* Force roundup */
		numeral((int) z / 10, &p);
		*p++ = '.';
		numeral((int) z % 10, &p);
		*p = 'm';
	} else
	if (z <= 999*T) {	/* 123m */
		numeral((int) ((z + T-1) / T), &p);
		*p = 'm';
	} else {		/* 1.2g */
		z= (z*10 + T*T-1) / (T*T);
		numeral((int) z / 10, &p);
		*p++ = '.';
		numeral((int) z % 10, &p);
		*p = 'g';
	}
	return siz;
}

/* Transform size of file to number of blocks.  This was once a function that
 * guessed the number of indirect blocks, but that nonsense has been removed.
 */
#if BSD
#define nblocks(f)	((f)->blocks)
#else
#define nblocks(f)	(((f)->size + BLOCK-1) / BLOCK)
#endif

/* From number of blocks to kilobytes. */
#if BLOCK < 1024
#define nblk2k(nb)	(((nb) + (1024 / BLOCK - 1)) / (1024 / BLOCK))
#else
#define nblk2k(nb)	((nb) * (BLOCK / 1024))
#endif

static int (*CMP)(struct file *f1, struct file *f2);
static int (*rCMP)(struct file *f1, struct file *f2);

static void mergesort(struct file **al)
/* This is either a stable mergesort, or thermal noise, I'm no longer sure.
 * It must be called like this: if (L != nil && L->next != nil) mergesort(&L);
 */
{
	/* static */ struct file *l1, **mid;  /* Need not be local */
	struct file *l2;

	l1= *(mid= &(*al)->next);
	do {
		if ((l1= l1->next) == nil) break;
		mid= &(*mid)->next;
	} while ((l1= l1->next) != nil);

	l2= *mid;
	*mid= nil;

	if ((*al)->next != nil) mergesort(al);
	if (l2->next != nil) mergesort(&l2);

	l1= *al;
	for (;;) {
		if ((*CMP)(l1, l2) <= 0) {
			if ((l1= *(al= &l1->next)) == nil) {
				*al= l2;
				break;
			}
		} else {
			*al= l2;
			l2= *(al= &l2->next);
			*al= l1;
			if (l2 == nil) break;
		}
	}
}

int namecmp(struct file *f1, struct file *f2)
{
	return strcmp(f1->name, f2->name);
}

int mtimecmp(struct file *f1, struct file *f2)
{
	return f1->mtime == f2->mtime ? 0 : f1->mtime > f2->mtime ? -1 : 1;
}

int atimecmp(struct file *f1, struct file *f2)
{
	return f1->atime == f2->atime ? 0 : f1->atime > f2->atime ? -1 : 1;
}

int ctimecmp(struct file *f1, struct file *f2)
{
	return f1->ctime == f2->ctime ? 0 : f1->ctime > f2->ctime ? -1 : 1;
}

int typecmp(struct file *f1, struct file *f2)
{
	return ifmt(f1->mode) - ifmt(f2->mode);
}

int revcmp(struct file *f1, struct file *f2) { return (*rCMP)(f2, f1); }

static void sort(struct file **al)
/* Sort the files according to the flags. */
{
	if (!present('f') && *al != nil && (*al)->next != nil) {
		CMP= namecmp;

		if (!(field & F_BYTIME)) {
			/* Sort on name */

			if (present('r')) { rCMP= CMP; CMP= revcmp; }
			mergesort(al);
		} else {
			/* Sort on name first, then sort on time. */

			mergesort(al);
			if (field & F_CTIME)
				CMP= ctimecmp;
			else
			if (field & F_ATIME)
				CMP= atimecmp;
			else
				CMP= mtimecmp;

			if (present('r')) { rCMP= CMP; CMP= revcmp; }
			mergesort(al);
		}
		/* Separate by file type if so desired. */

		if (field & F_TYPE) {
			CMP= typecmp;
			mergesort(al);
		}
	}
}

struct file *newfile(char *name)
/* Create file structure for given name. */
{
	struct file *new;

	new= (struct file *) allocate(sizeof(*new));
	new->name= strcpy((char *) allocate(strlen(name)+1), name);
	return new;
}

void pushfile(struct file **flist, struct file *new)
/* Add file to the head of a list. */
{
	new->next= *flist;
	*flist= new;
}

void delfile(struct file *old)
/* Release old file structure. */
{
	free((void *) old->name);
	free((void *) old);
}

struct file *popfile(struct file **flist)
/* Pop file off top of file list. */
{
	struct file *f;

	f= *flist;
	*flist= f->next;
	return f;
}

int dotflag(char *name)
/* Return flag that would make ls list this name: -a or -A. */
{
	if (*name++ != '.') return 0;

	switch (*name++) {
	case 0:		return 'a';			/* "." */
	case '.':	if (*name == 0) return 'a';	/* ".." */
	default:	return 'A';			/* ".*" */
	}
}

int adddir(struct file **aflist, char *name)
/* Add directory entries of directory name to a file list. */
{
	DIR *d;
	struct dirent *e;

	if (access(name, 0) < 0) {
		report(name);
		return 0;
	}

	if ((d= opendir(name)) == nil) {
		report(name);
		return 0;
	}
	while ((e= readdir(d)) != nil) {
		if (e->d_ino != 0 && present(dotflag(e->d_name))) {
			pushfile(aflist, newfile(e->d_name));
			aflist= &(*aflist)->next;
		}
	}
	closedir(d);
	return 1;
}

off_t countblocks(struct file *flist)
/* Compute total block count for a list of files. */
{
	off_t cb = 0;

	while (flist != nil) {
		switch (flist->mode & S_IFMT) {
		case S_IFDIR:
		case S_IFREG:
#ifdef S_IFLNK
		case S_IFLNK:
#endif
			cb += nblocks(flist);
		}
		flist= flist->next;
	}
	return cb;
}

void printname(char *name)
/* Print a name with control characters as '?' (unless -q).  The terminal is
 * assumed to be eight bit clean.
 */
{
	int c, q= present('q');

	while ((c= *name++) != 0) {
		if (q && (c <= ' ' || c == 0177)) c= '?';
		putchar(c);
	}
}

int mark(struct file *f, int doit)
{
	int c;

	if (!(field & F_MARK)) return 0;

	switch (f->mode & S_IFMT) {
	case S_IFDIR:	c= '/'; break;
#ifdef S_IFIFO
	case S_IFIFO:	c= '|'; break;
#endif
#ifdef S_IFLNK
	case S_IFLNK:	c= '@'; break;
#endif
#ifdef S_IFSOCK
	case S_IFSOCK:	c= '='; break;
#endif
	case S_IFREG:
		if (f->mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
			c= '*';
			break;
		}
	default:
		c= 0;
	}
	if (doit && c != 0) putchar(c);
	return c;
}

int colwidth[MAXCOLS];	/* Need colwidth[i] spaces to print column i. */
int sizwidth[MAXCOLS];	/* Spaces for the size field in a -X print. */
int namwidth[MAXCOLS];	/* Name field. */

int maxise(int *aw, int w)
/* Set *aw to the larger of it and w.  Then return it. */
{
	if (w > *aw) *aw= w;
	return *aw;
}

static int nsp= 0;	/* This many spaces have not been printed yet. */
#define spaces(n)	(nsp= (n))
#define terpri()	(nsp= 0, putchar('\n'))		/* No trailing spaces */

void print1(struct file *f, int col, int doit)
/* Either compute the number of spaces needed to print file f (doit == 0) or
 * really print it (doit == 1).
 */
{
	int width= 0, n;
	char *p;

	while (nsp>0) { putchar(' '); nsp--; }/* Fill gap between two columns */

	if (field & F_INODE) {
		if (doit) printf("%5d ", f->ino); else width+= 6;
	}
	if (field & F_BLOCKS) {
		if (doit) printf("%4ld ", nblk2k(nblocks(f))); else width+= 5;
	}
	if (field & F_MODE) {
		if (doit)
			printf("%s ", permissions(f));
		else
			width+= (field & F_EXTRA) ? 5 : 11;
	}
	if (field & F_EXTRA) {
		p= cxsize(f);
		n= strlen(p)+1;

		if (doit) {
			n= sizwidth[col] - n;
			while (n > 0) { putchar(' '); --n; }
			printf("%s ", p);
		} else
			width+= maxise(&sizwidth[col], n);
	}
	if (field & F_LONG) {
		if (doit) {
			printf("%2d %-8s ", f->nlink, uidname(f->uid));
			if (field & F_GROUP) printf("%-8s ", gidname(f->gid));

			switch (f->mode & S_IFMT) {
			case S_IFBLK:
			case S_IFCHR:
#ifdef S_IFMPB
			case S_IFMPB:
#endif
#ifdef S_IFMPC
			case S_IFMPC:
#endif
				printf("%3d, %3d ",
					major(f->rdev), minor(f->rdev));
				break;
			default:
				printf("%8ld ", (long) f->size);
			}
			printf("%s ", timestamp(f));
		} else
			width += (field & F_GROUP) ? 43 : 34;
	}
	n= strlen(f->name);
	if (doit) {
		printname(f->name);
		if (mark(f, 1) != 0) n++;
#ifdef S_IFLNK
		if ((field & F_LONG) && (f->mode & S_IFMT) == S_IFLNK) {
			char *buf;
			int r, didx;

			buf= (char *) allocate(((size_t) f->size + 1)
							* sizeof(buf[0]));
			addpath(&didx, f->name);
			r= readlink(path, buf, (int) f->size);
			delpath(didx);
			if (r > 0) buf[r] = 0; else r=1, strcpy(buf, "?");
			printf(" -> ");
			printname(buf);
			free((void *) buf);
			n+= 4 + r;
		}
#endif
		spaces(namwidth[col] - n);
	} else {
		if (mark(f, 0) != 0) n++;
#ifdef S_IFLNK
		if ((field & F_LONG) && (f->mode & S_IFMT) == S_IFLNK) {
			n+= 4 + (int) f->size;
		}
#endif
		width+= maxise(&namwidth[col], n + NSEP);
		maxise(&colwidth[col], width);
	}
}

int countfiles(struct file *flist)
/* Return number of files in the list. */
{
	int n= 0;

	while (flist != nil) { n++; flist= flist->next; }

	return n;
}

struct file *filecol[MAXCOLS];	/* filecol[i] is list of files for column i. */
int nfiles, nlines;	/* # files to print, # of lines needed. */

int columnise(struct file *flist, int nplin)
/* Chop list of files up in columns.  Note that 3 columns are used for 5 files
 * even though nplin may be 4, filecol[3] will simply be nil.
 */
{
	int i, j;

	nlines= (nfiles + nplin - 1) / nplin;	/* nlines needed for nfiles */

	filecol[0]= flist;

	for (i=1; i<nplin; i++) {	/* Give nlines files to each column. */
		for (j=0; j<nlines && flist != nil; j++) flist= flist->next;

		filecol[i]= flist;
	}
}

int print(struct file *flist, int nplin, int doit)
/* Try (doit == 0), or really print the list of files over nplin columns.
 * Return true if it can be done in nplin columns or if nplin == 1.
 */
{
	register struct file *f;
	register int i, totlen;

	columnise(flist, nplin);

	if (!doit) {
		if (nplin==1 && !(field & F_EXTRA))
			return 1;	/* No need to try 1 column. */

		for (i=0; i<nplin; i++)
			colwidth[i]= sizwidth[i]= namwidth[i]= 0;
	}
	while (--nlines >= 0) {
		totlen=0;

		for (i=0; i<nplin; i++) {
			if ((f= filecol[i]) != nil) {
				filecol[i]= f->next;
				print1(f, i, doit);
			}
			if (!doit && nplin>1) {
				/* See if this line is not too long. */
				totlen+= colwidth[i];
				if (totlen > ncols+NSEP) return 0;
			}
		}
		if (doit) terpri();
	}
	return 1;
}

enum depth { SURFACE, SURFACE1, SUBMERGED };
enum state { BOTTOM, SINKING, FLOATING };

void listfiles(struct file *flist, enum depth depth, enum state state)
/* Main workhorse of ls, it sorts and prints the list of files.  Flags:
 * depth: working with the command line / just one file / listing dir.
 * state: How "recursive" do we have to be.
 */
{
	struct file *dlist= nil, **afl= &flist, **adl= &dlist;
	int nplin;
	static int white = 1;	/* Nothing printed yet. */

	/* Flush everything previously printed, so new error output will
	 * not intermix with files listed earlier.
	 */
	fflush(stdout);

	if (field != 0 || state != BOTTOM) {	/* Need stat(2) info. */
		while (*afl != nil) {
			static struct stat st;
			int r, didx;

			addpath(&didx, (*afl)->name);

			if ((r= status(path, &st)) < 0
#ifdef S_IFLNK
				&& (status == lstat || lstat(path, &st) < 0)
#endif
			) {
				if (depth != SUBMERGED || errno != ENOENT)
					report((*afl)->name);
				delfile(popfile(afl));
			} else {
				setstat(*afl, &st);
				afl= &(*afl)->next;
			}
			delpath(didx);
		}
	}
	sort(&flist);

	if (depth == SUBMERGED && (field & (F_BLOCKS | F_LONG)))
		printf("total %ld\n", nblk2k(countblocks(flist)));

	if (state == SINKING || depth == SURFACE1) {
	/* Don't list directories themselves, list their contents later. */
		afl= &flist;
		while (*afl != nil) {
			if (((*afl)->mode & S_IFMT) == S_IFDIR) {
				pushfile(adl, popfile(afl));
				adl= &(*adl)->next;
			} else
				afl= &(*afl)->next;
		}
	}

	if ((nfiles= countfiles(flist)) > 0) {
		/* Print files in how many columns? */
		nplin= !present('C') ? 1 : nfiles < MAXCOLS ? nfiles : MAXCOLS;

		while (!print(flist, nplin, 0)) nplin--;	/* Try first */

		print(flist, nplin, 1);		/* Then do it! */
		white = 0;
	}

	while (flist != nil) {	/* Destroy file list */
		if (state == FLOATING && (flist->mode & S_IFMT) == S_IFDIR) {
			/* But keep these directories for ls -R. */
			pushfile(adl, popfile(&flist));
			adl= &(*adl)->next;
		} else
			delfile(popfile(&flist));
	}

	while (dlist != nil) {	/* List directories */
		if (dotflag(dlist->name) != 'a' || depth != SUBMERGED) {
			int didx;

			addpath(&didx, dlist->name);

			flist= nil;
			if (adddir(&flist, path)) {
				if (depth != SURFACE1) {
					if (!white) putchar('\n');
					printf("%s:\n", path);
					white = 0;
				}
				listfiles(flist, SUBMERGED,
					state == FLOATING ? FLOATING : BOTTOM);
			}
			delpath(didx);
		}
		delfile(popfile(&dlist));
	}
}

int main(int argc, char **argv)
{
	struct file *flist= nil, **aflist= &flist;
	enum depth depth;
	char *lsflags;
#ifdef TIOCGWINSZ
	struct winsize ws;
#endif

	uid= geteuid();
	gid= getegid();

	if ((arg0= strrchr(argv[0], '/')) == nil) arg0= argv[0]; else arg0++;
	argv++;

	if (strcmp(arg0, "ls") != 0) {
		char *p= arg0+1;

		while (*p != 0) {
			if (strchr(arg0flag, *p) != nil) *p += 'A' - 'a';
			p++;
		}
		setflags(arg0+1);
	}
	while (*argv != nil && (*argv)[0] == '-') {
		if ((*argv)[1] == '-' && (*argv)[2] == 0) {
			argv++;
			break;
		}
		setflags(*argv++ + 1);
	}

	istty= isatty(1);

	if (istty && (lsflags= getenv("LSOPTS")) != nil) {
		if (*lsflags == '-') lsflags++;
		setflags(lsflags);
	}

	if (!present('1') && !present('C') && !present('l')
		&& (istty || present('M') || present('X') || present('F'))
	) setflags("C");

	if (istty) setflags("q");

	if (SUPER_ID == 0 || present('a')) setflags("A");

	if (present('i')) field|= F_INODE;
	if (present('s')) field|= F_BLOCKS;
	if (present('M')) field|= F_MODE;
	if (present('X')) field|= F_EXTRA|F_MODE;
	if (present('t')) field|= F_BYTIME;
	if (present('u')) field|= F_ATIME;
	if (present('c')) field|= F_CTIME;
	if (present('l')) {
		field= (field | F_MODE | F_LONG) & ~F_EXTRA;
		if (present('g')) field|= F_GROUP;
	}
	if (present('F')) field|= F_MARK;
	if (present('T')) field|= F_TYPE;
	if (present('d')) field|= F_DIR;

#ifdef S_IFLNK
	status= present('L') ? stat : lstat;
#endif

#ifdef TIOCGWINSZ
	if (present('C')) {
		int t= istty ? 1 : open("/dev/tty", O_WRONLY);

		if (t >= 0 && ioctl(t, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
			ncols= ws.ws_col - 1;

		if (t != 1) close(t);
	}
#endif

	depth= SURFACE;

	if (*argv == nil) {
		if (!(field & F_DIR)) depth= SURFACE1;
		pushfile(aflist, newfile("."));
	} else {
		if (argv[1] == nil && !(field & F_DIR)) depth= SURFACE1;

		do {
			pushfile(aflist, newfile(*argv++));
			aflist= &(*aflist)->next;
		} while (*argv!=nil);
	}
	listfiles(flist, depth,
		(field & F_DIR) ? BOTTOM : present('R') ? FLOATING : SINKING);
	exit(ex);
}
/* Kees J. Bot  25-4-89. */
