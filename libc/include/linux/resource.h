/*
 * Resource control/accounting header file for linux-86
 */

#ifndef __LINUX_RESOURCE_H
#define __LINUX_RESOURCE_H

#include <features.h>
#include <sys/time.h>
#include <limits.h>

#define	RUSAGE_SELF	0
#define	RUSAGE_CHILDREN	(-1)
#define RUSAGE_BOTH	(-2)		/* sys_wait4() uses this */

struct	rusage {
	struct timeval ru_utime;	/* user time used */
	struct timeval ru_stime;	/* system time used */
	long	ru_maxrss;		/* maximum resident set size */
	long	ru_ixrss;		/* integral shared memory size */
	long	ru_idrss;		/* integral unshared data size */
	long	ru_isrss;		/* integral unshared stack size */
	long	ru_minflt;		/* page reclaims */
	long	ru_majflt;		/* page faults */
	long	ru_nswap;		/* swaps */
	long	ru_inblock;		/* block input operations */
	long	ru_oublock;		/* block output operations */
	long	ru_msgsnd;		/* messages sent */
	long	ru_msgrcv;		/* messages received */
	long	ru_nsignals;		/* signals received */
	long	ru_nvcsw;		/* voluntary context switches */
	long	ru_nivcsw;		/* involuntary " */
};

#define RLIM_INFINITY	((long)(~0UL>>1))

struct rlimit {
	long	rlim_cur;
	long	rlim_max;
};

#define	PRIO_MIN	(-20)
#define	PRIO_MAX	20

#define	PRIO_PROCESS	0
#define	PRIO_PGRP	1
#define	PRIO_USER	2

#define RLIMIT_CPU	0		/* CPU time in ms */
#define RLIMIT_FSIZE	1		/* Maximum filesize */
#define RLIMIT_DATA	2		/* max data size */
#define RLIMIT_STACK	3		/* max stack size */
#define RLIMIT_CORE	4		/* max core file size */
#define RLIMIT_RSS	5		/* max resident set size */
#define RLIMIT_NPROC	6		/* max number of processes */
#define RLIMIT_NOFILE	7		/* max number of open files */
#define RLIMIT_MEMLOCK	8		/* max locked-in-memory address space */

#define RLIM_NLIMITS	9

extern int	getrlimit __P ((int __resource,
			struct rlimit *__rlp));
extern int	setrlimit __P ((int __resource,
			__const struct rlimit *__rlp));

extern int      getpriority __P((int __which, int __who));
extern int      setpriority __P((int __which, int __who,
			int __prio));

extern int	__getrusage __P ((int __who, struct rusage *__rusage));
extern int	getrusage __P ((int __who, struct rusage *__rusage));

#endif /* __LINUX_RESOURCE_H */
