
#include "doselks.h"
#include "syscalls.h"

/***************************************************************************/

/* Need to use stdio for CR<->NL translation */
#define MAX_FD 20
FILE * tty_file[MAX_FD] = {stdin, stdout, stderr};

int nextpid = 2;
int proc_one = 1;

/***************************************************************************/

#define sys_exit elks_exit
elks_exit(wait_stat)
int wait_stat;
{
   struct process * cur;
   int x;

   if(wait_stat & 0x80) dump_core();

   /* save wait_stat */
   proc->ax = wait_stat;
   if(proc->pid == 1) proc_one = 0;

   process_running--;
   seg_free(proc->text_seg);
   seg_free(proc->data_seg);

   for(cur=proc->next; cur!=proc; cur=cur->next)
      if(cur->ppid == proc->pid)
         cur->ppid = 1;

   return 0;
}

#define sys_fork elks_fork
elks_fork(bx, cx, dx)
int bx, cx, dx;
{
   unsigned int size, i;
   struct process * newproc;

   if(nextpid<0) return -EINVAL;	/* TODO: max 32k forks */

   newproc = calloc(sizeof(struct process), 1);
   if( newproc == 0 ) return -ENOMEM;

   proc->pending_op = 0;
   *newproc = *proc;
   newproc->ppid = newproc->pid;
   newproc->data_seg = seg_allocate(proc->data_seg->size);

   if( newproc->data_seg == 0 )
   {
      free(newproc);
      return -ENOMEM;
   }
   newproc->text_seg->ref_count++;

   newproc->cs = newproc->text_seg->seg;
   newproc->ds = newproc->data_seg->seg;
   newproc->ss = newproc->ds;

   newproc->pid = nextpid++;

   /* Duplicate memory */
   for(i=0; i<newproc->data_seg->size; i+=size )
   {
      size = 0xFFF;
      if(size > newproc->data_seg->size-i) size = newproc->data_seg->size-i;

      __movedata(proc->ds, i*16, newproc->ds, i*16, size*16);
   }

   proc->ax = newproc->pid;
   newproc->ax = 0;
   proc->next = newproc;
   process_running++;
   /* proc = newproc; */

   return newproc->pid;
}

struct process *
get_zombie(ppid, pid)
int ppid, pid;
{
   struct process * prev, * cur, *running=0;
   /* Process group, not defined */
   if( pid < -1 ) return 0;
   if( pid == 0 ) pid = -1;

   for(prev=proc, cur=proc->next; cur!=proc; prev=cur, cur=cur->next)
   {
      if(cur->ppid == ppid && (pid == -1 || cur->pid == pid))
      {
	 if(cur->pending_op != 1)
	 {
	     running = cur;
	     continue;
	 }
	 prev->next = cur->next;
         cur->next = 0;
	 return cur;
      }
   }
   return running;
}

#define sys_wait4 elks_wait4
elks_wait4(bx, cx, dx, di, si)
int bx, cx, dx, di, si;
{
   struct process * cur;
   int cpid;

   cur = get_zombie(proc->pid, bx);
   if( cur == 0 ) return -ECHILD;

   if( cur->pending_op != 1)
   {
      if( dx & WNOHANG )
         return 0;
      else
	 return -EINTR;
   }

   cpid = cur->pid;
   if( cx ) __doke_es(cx, cur->ax);
   if( di )
      __movedata(current_ds, (unsigned)&(cur->ruse),
                 proc->ds, di, sizeof(struct rusage));
   free(cur);
   return cpid;
}

#define sys_exec elks_exec
int elks_exec(bx, cx, dx)
int bx, cx, dx;
{
   struct process oldproc;
   char fname[128];
   int ax;

   oldproc = *proc;
   __strnget_es(fname, bx, sizeof(fname)-1);
   fname[sizeof(fname)-1] = '\0';

   if( (ax=loadexe(fname)) != 0 )
   {
      *proc = oldproc;
      return ax;
   }

   __movedata(oldproc.ds, cx, proc->ds, proc->sp-dx, dx);

   /*
      Fudge pointers.
   */
   proc->sp -= dx;

   /* Args */
   ax = proc->sp + 2;
   __set_es(proc->ds);
   while(__deek_es(ax) != 0 )
       __doke_es(ax, __deek_es(ax)-dx), ax+=2;

   /* env */
   ax+=2;
   while(__deek_es(ax) != 0 )
       __doke_es(ax, __deek_es(ax)-dx), ax+=2;

   seg_free(oldproc.text_seg);
   seg_free(oldproc.data_seg);
   return 0;
}

#define sys_read elks_read
int elks_read(bx, cx, dx)
int bx, cx, dx;
{
   register int ch, rv=0;
   if( bx<0 || bx>MAX_FD || tty_file[bx] == 0 ) return -EBADF;
   do
   {
      ch = fgetc(tty_file[bx]);
      if( ch == EOF ) break;
      __poke_es(cx++, ch);
      rv++;
   }
   while(--dx > 0 && stdio_pending(tty_file[bx]));

   return rv;
}

#define sys_write elks_write
int elks_write(bx, cx, dx)
unsigned int bx, cx, dx;
{
   char ch[2];
   register unsigned int i;
   if( bx<0 || bx>MAX_FD || tty_file[bx] == 0 ) return -EBADF;

   for(i=dx; i>0; i--)
   {
      if( fputc(__peek_es(cx++), tty_file[bx]) == EOF )
         return -1;
   }
   fflush(tty_file[bx]);
   return dx;
}

#define sys_open elks_open
int elks_open(bx, cx, dx)
{
   char fname[128];
   int i;
   char *mode = "";

   __strnget_es(fname, bx, sizeof(fname)-1);
   fname[sizeof(fname)-1] = '\0';

   for(i=0; i<MAX_FD; i++)
   {
      if(tty_file[i] == 0) break;
   }
   if( i >= MAX_FD ) return -EMFILE;

   /* Translate the mode; more or less */
   switch(cx&O_ACCMODE)
   {
   case O_RDONLY:
      mode="r";
      break;
   case O_WRONLY:
      if( cx&O_APPEND )      mode="a";
      else if( cx&O_TRUNC )  mode="w";
      else                   mode="r+";
      break;
   case O_RDWR:
      if( cx&O_APPEND )     mode="a+";
      else if( cx&O_TRUNC ) mode="w+";
      else                  mode="r+";
      break;
   }

   if( ( tty_file[i] = fopen(fname, mode) ) == 0 )
   {
      if( errno == 0 ) return -ENOENT;
      return -errno;
   }
   return i;
}

#define sys_close elks_close
int elks_close(bx, cx, dx)
{
   if( bx<0 || bx>MAX_FD || tty_file[bx] == 0 ) return -EBADF;
   fclose(tty_file[bx]);
   tty_file[bx] = 0;
   return 0;
}

#define sys_lseek elks_lseek
int elks_lseek(bx, cx, dx)
{
   long l;
   if( bx<0 || bx>MAX_FD || tty_file[bx] == 0 ) return -EBADF;

   __movedata(proc->ds, cx, current_ds, &l, 4);

   if( fseek(tty_file[bx], l, dx) < 0 ) return -ESPIPE;

   l = ftell(tty_file[bx]);
   __movedata(current_ds, &l, proc->ds, cx, 4);
   return 0;
}

#define sys_brk elks_brk
int elks_brk(bx, cx, dx)
{
   /* For the moment we don't care; libc does enough */
   return 0;
}

#define sys_getpid elks_getpid
int elks_getpid(bx, cx, dx)
{
   __doke_es(bx, proc->ppid);
   return proc->pid;
}

#define sys_getuid elks_getuid
int elks_getuid(bx, cx, dx)
{
   __doke_es(bx, proc->euid);
   return proc->uid;
}

#define sys_getgid elks_getgid
int elks_getgid(bx, cx, dx)
{
   __doke_es(bx, proc->egid);
   return proc->gid;
}

#define sys_setuid elks_setuid
int elks_setuid(bx, cx, dx)
{
   if( proc->euid == 0
    || proc->uid  == bx || proc->euid == bx || proc->suid == bx)
   {
      if( proc->euid == 0 )
         proc->suid = proc->uid = bx;
      proc->euid = bx;
   }
   else
      return -EPERM;
}

#define sys_setgid elks_setgid
int elks_setgid(bx, cx, dx)
{
   if( proc->euid == 0
    || proc->gid  == bx || proc->egid == bx || proc->sgid == bx)
   {
      if( proc->euid == 0 )
         proc->sgid = proc->gid = bx;
      proc->egid = bx;
   }
   else
      return -EPERM;
}

#define sys_umask elks_umask
int elks_umask(bx, cx, dx)
{
   cx= proc->umask;
   proc->umask = (bx&0777);
   return cx;
}

void sig_trap(signo)
int signo;
{
   proc->sp -= 2;
   __doke_es(proc->sp, signo);
   proc->sp -= 2;
   __doke_es(proc->sp, proc->pc);
   proc->pc = proc->sigtrap;
}

#define sys_signal elks_signal
int elks_signal(bx,cx,dx)
{
   int rv;
   if( bx < 0 || bx >= NSIG ) return -EINVAL;
   if( bx == SIGSTOP || bx == SIGKILL ) return 0;

   proc->trpmask &= ~(1L<<bx);
   proc->ignmask &= ~(1L<<bx);

   if( cx == 0 )      /* Default */;
   else if( cx == 1 ) proc->ignmask |= (1L<<bx);
   else 
   {
      proc->trpmask |= (1L<<bx);
      proc->sigtrap = cx;
   }

   return 0;
}

#define sys_lstat elks_stat
#define sys_stat elks_stat
int elks_stat(bx,cx,dx)
{
   struct stat ms;
   char fnamebuf[65];
   char dtabuf[128];	/* I think this only needs 32 bytes ... but ... */
   int v;
   long ttime, tdate;

   __strnget_es(fnamebuf, bx, sizeof(fnamebuf)-1);
   fnamebuf[64] = '\0';

   v = __dos_stat(fnamebuf, dtabuf);
   if( v < 0 ) return -ENOENT;

   ms.st_dev=   *dtabuf;
   ms.st_ino=   0;
   ms.st_mode=  ((dtabuf[0x15]&0x1)?0555:0777);  /* + File or directory */
   if (dtabuf[0x15]&0x10)
      ms.st_mode |= S_IFDIR;
   else
      ms.st_mode |= S_IFREG;
   ms.st_nlink= 1;
   ms.st_uid=   0;
   ms.st_gid=   0;
   ms.st_rdev=  0;
   ms.st_size=  *(long*)(dtabuf+0x1A);

   ttime = *((unsigned short*)(dtabuf+0x16));
   ttime = ( ttime      &0x1F) * 2
         + ((ttime>>5)  &0x2F) * 60
         + ((ttime>>11) &0x1F) * 3600L;

/* FIXME: This needs a julian day calculator .. */
   tdate = *((unsigned short*)(dtabuf+0x18));
   tdate = ((tdate)     &0x1F) -32
         + ((tdate>>5)  &0x0F) * 31
         + ((tdate>>9)  &0x7F) * (31*12);

   ms.st_mtime= tdate*86400L + ttime;
   ms.st_atime= ms.st_mtime;
   ms.st_ctime= ms.st_mtime;

   /* copy &ms -> es:cx */
   __movedata(current_ds, &ms, proc->ds, cx, sizeof(struct stat));

   return 0;
}

/***************************************************************************/

#define sys_enosys elks_enosys
int elks_enosys(bx, cx, dx)
int bx, cx, dx;
{
   printf("Unimplemented syscall(%d,0x%04x,0x%04x,0x%04x)\n",
	   proc->ax,proc->bx,proc->cx,proc->dx);
   return -ENOSYS;
}

#include "defn_tab.v"

Scallp syscalltab[] = {
#include "call_tab.v"
   elks_enosys
};

FILE * error;
run_executable()
{

   register unsigned int r_ax;
   register Scallp ptr;
   process_running=1;

   error = fopen("errlog", "w");

   do
   {
      if( proc->pending_op == 0 )
      {
         fprintf(error, "Run %d AX=%-4d proc =%04x IP=%04x...",
	         proc->pid, proc->ax, proc, proc->pc); fflush(error);
         proc->pending_op = run_cpu();
      }
      if( r_ax = proc->pending_op )
      {
         if( r_ax >= sizeof(syscalltab)/sizeof(Scallp) )
            r_ax = 0;

	 fprintf(error, "%d: Call_%d(%04x, %04x, %04x) -> ",
	         proc->pid, r_ax, proc->bx, proc->cx, proc->dx);
	 fflush(error);

         if( r_ax == 1 )
            (void) elks_exit(proc->bx << 8);
	 else
	 {
            __set_es(proc->ds);
            if( (proc->ax = (*syscalltab[r_ax])(proc->bx, proc->cx, proc->dx,
                                                proc->di, proc->si))
                != (unsigned) -EINTR )
            {
               proc->pending_op = 0;
	       /* 
	       fprintf(error, "%d\n", proc->ax);
	       fflush(error);
               continue;
	       */
            }
	 }
	 fprintf(error, "%d\n", proc->ax);
	 fflush(error);
      }
      intr_pending = 0;	/* FIXME: deal with */
      do { proc = proc->next; } while(proc->pending_op==1 && process_running);
   }
   while(process_running);
}

