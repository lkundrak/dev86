
#include <features.h>
/* Sneaky, use the _linuxmt_ header files _not_ dos */
#undef  __SYSINC__
#define __SYSINC__(__h_file) <linuxmt/__h_file>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/errno.h>

#include <stdio.h>
#include <ctype.h>
#include <dos.h>
#include <malloc.h>
#include <fcntl.h>

/* Function return vals */

extern unsigned int elks_int();
extern unsigned int run_cpu();

/* Elks binary formats */
 
#define EXEC_HEADER_SIZE	32

struct elks_exec_hdr
{
	unsigned long type;
#define ELKS_COMBID	0x04100301L
#define ELKS_SPLITID	0x04200301L	
	unsigned long hlen;
	unsigned long tseg;
	unsigned long dseg;
	unsigned long bseg;
	unsigned long unused;
	unsigned long chmem;
	unsigned long unused2; 
};

/* Elks cpu structure. */

struct fd_ref {
    int ftype;
    union {
       int i;
       FILE * f;
    } fd;
} ;

struct segment {
   unsigned int seg;
   unsigned int size;
   int ref_count;
};

struct process
{
   unsigned int ax;	/* NOTE The order of this is _closely_ related to */
   unsigned int bx;	/* the coroutine assembler */
   unsigned int cx;
   unsigned int dx;
   unsigned int di;
   unsigned int si;
   unsigned int bp;
   unsigned int es;

   unsigned int ds;	/* start +16 +0 */
   unsigned int pc;	/* start +16 +2 */
   unsigned int cs;	/* start +16 +4 */
   unsigned int flgs;	/* start +16 +6 */
   unsigned int ss;	/* start +16 +8 */
   unsigned int sp;	/* start +16 +10 */

   /* From here down only accessed from C */
   int pending_op;
   struct process * next;

   struct segment * text_seg;
   struct segment * data_seg;

   int pid;
   int ppid;
   int pgrp;
   int umask;
   int uid;
   int gid;
   int euid;
   int egid;
   int suid;
   int sgid;
   long ignmask, trpmask;
   int sigtrap;

   struct fd_ref fds[20];

   struct rusage ruse;
};

extern struct process * proc;
extern unsigned int current_ds, current_stack;
extern int in_process;
extern int process_running;
extern int intr_pending;

/* Elks syscall interface */

#define ELKSINT   0x80
typedef int (*Scallp)( /* int, int, int */ );
extern Scallp syscalltab[];

/******************************************************************************/

#ifndef O_BINARY
#define O_BINARY 0
#endif

