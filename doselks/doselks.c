
#include "doselks.h"

#define DEBUGGER

void build_stack(/* char ** argv, char ** envp */);

long elks_int_orig;
union  REGS regs;
int process_running;
int exit_code = 255;

main(argc, argv, envp)
int argc;
char ** argv;
char ** envp;
{
   int rv;

   current_ds = __get_ds();
   if( argc <= 1 )
   {
      printf("Usage: %s <8086_exe> [args]\n", argv[0]);
      exit(exit_code);
   }

   proc = calloc(sizeof(struct process), 1);
   if( proc == 0 )
         printf("Unable to allocate memory for executable\n");
   else if( (rv = loadexe(argv[1])) >= 0 )
   {
      proc->pid = 1;
      proc->ppid = 1;
      proc->pgrp = 1;
      proc->umask = 022;
      proc->next = proc;

      build_stack(argv+1, envp);

      elks_int_orig = __getvect(ELKSINT);
      __setvect(ELKSINT, (long)elks_int);

#ifndef MAKEME
      run_executable();
#else
      dump_core();
#endif

      __setvect(ELKSINT, elks_int_orig);
   }
   else
   {
      if( rv == -ENOMEM )
         printf("Unable to allocate memory for executable\n");
      else
         printf("Cannot execute %s\n", argv[1]);
   }
   return(exit_code);
}

int loadexe(fname)
char * fname;
{
	char bbuf[1024];
	long size;
	unsigned int tsize, dsize;
	struct elks_exec_hdr mh;
	int i;
	int fd = -1;
	int err = ENOEXEC;

	proc->text_seg = 0;
	proc->data_seg = 0;

	if( ( fd = open(fname, O_RDONLY|O_BINARY) ) < 0 )
	   goto no_exec;

	/* Load the elks binary image and set it up in a suitable
	 * segment. Load CS and DS/SS according to image type.
	 */

	if( read(fd, &mh,sizeof(mh))!=sizeof(mh)
	 || mh.hlen!=EXEC_HEADER_SIZE
	 || (mh.type!=ELKS_COMBID&&mh.type!=ELKS_SPLITID)
         )
	 	goto no_exec;

	if(mh.type==ELKS_COMBID)
	{
		tsize = 0;
		dsize = ((mh.tseg+mh.chmem + 0xF)>>4);
	}
	else
	{
		tsize = ((mh.tseg+0xF) >> 4); 
		dsize = ((mh.chmem+0xF) >> 4); 
	}
	if(tsize)
	{
           if( (proc->text_seg = seg_allocate(tsize)) == 0)
	      goto no_mem;

	   proc->cs = proc->text_seg->seg;
        }

   	if( (proc->data_seg = seg_allocate(dsize)) == 0 )
	   goto no_mem;

	proc->ds = proc->data_seg->seg;
	if(proc->text_seg == 0) proc->cs = proc->ds;

#ifdef DEBUGGER
	fprintf(stderr,"Executable - %s. tseg=0x%04lx dseg=0x%04lx bss=0x%04lx chmem=0x%04lx\n",
		mh.type==ELKS_COMBID?"(impure)":"Sep I/D",
		mh.tseg,mh.dseg,mh.bseg,mh.chmem);
#endif

        if(proc->text_seg)
	{
	   for(size=0; size<mh.tseg; )
	   {
		i = sizeof(bbuf); if( i>mh.tseg-size ) i=mh.tseg-size;

		if(read(fd,bbuf,i)!=i)
		   goto no_exec;

	   	__movedata(current_ds, (unsigned)bbuf, proc->cs, (int)size, i);
	   	size += i;
	   }
	}
	else
	   mh.dseg += mh.tseg;

	for(size=0; size<mh.dseg; )
	{
	     i = sizeof(bbuf); if( i>mh.dseg-size ) i=mh.dseg-size;

	     if(read(fd,bbuf,i)!=i)
		goto no_exec;
	     
	     __movedata(current_ds, (unsigned)bbuf, proc->ds, (int)size, i);
	     size += i;
	}

	close(fd);

	/* Zap the bss */
	memset(bbuf, '\0', 1024);
	mh.dseg += mh.bseg;
	for(; size<mh.dseg; )
	{
	     i = sizeof(bbuf); if( i>mh.dseg-size ) i=mh.dseg-size;

	     __movedata(current_ds, (unsigned)bbuf, proc->ds, (int)size, i);
	     size += i;
	}

	/*
	 *	Load the VM86 registers
	 */
	 
	proc->ss=proc->ds;
	proc->es=0;		/* This will be preserved */
	proc->sp=mh.chmem; 	/* Args stacked later */
	proc->pc=0;		/* Run from 0 */
	
	/*
	 *	Loaded
	 */

	return 0;
no_mem:;
	err = ENOMEM;
no_exec:;
	seg_free(proc->text_seg);
	seg_free(proc->data_seg);
	if( fd > 0 ) close(fd);
	return -err;
}

void
build_stack(argv, envp)
char ** argv; char ** envp;
{
	char buffer[1024];

	char **p;
	int argv_len=0, argv_count=0;
	int envp_len=0, envp_count=0;
	int stack_bytes;
	unsigned short * pip;
	char * pcp;

	/* How much space for argv */
	if(argv) for(p=argv; *p; p++)
	{
	   argv_count++; argv_len += strlen(*p)+1;
	}

	/* How much space for envp */
	if(envp) for(p=envp; *p; p++)
	{
	   envp_count++; envp_len += strlen(*p)+1;
	}

	/* tot it all up */
	stack_bytes = 2				/* argc */
	            + argv_count * 2 + 2	/* argv */
		    + argv_len
		    + envp_count * 2 + 2	/* envp */
		    + envp_len;

	if( stack_bytes > sizeof(buffer)-2 )
	{
		printf("Environment too large\n");
		exit(exit_code);
	}
	/* Allocate it */
	proc->sp -= stack_bytes;

#ifdef DEBUGGER
	printf("Argv = (%d,%d), Envp=(%d,%d), stack=%d\n",
	        argv_count, argv_len, envp_count, envp_len, stack_bytes);
#endif

	/* Now build the data */
	pip=(unsigned short *) buffer;
	pcp=buffer+sizeof(unsigned short *)*(1+argv_count+1+envp_count+1);

	*pip++ = argv_count;
	for(p=argv; *p; p++)
	{
	   *pip++ = (pcp-buffer)+proc->sp;
	   strcpy(pcp, *p);
	   pcp += strlen(*p)+1;
	}
	*pip++ = 0;

	for(p=envp; *p; p++)
	{
	   *pip++ = (pcp-buffer)+proc->sp;
	   strcpy(pcp, *p);
	   pcp += strlen(*p)+1;
	}
	*pip++ = 0;
	__movedata(current_ds, (unsigned)buffer,
                 proc->ds, proc->sp, stack_bytes);
}

/***************************************************************************/

dump_core()
{
	char bbuf[1024];
	int i, fd;
	unsigned destseg;
	printf("Not dumping core\n");

/*
	if( ( fd = open("core", O_WRONLY|O_BINARY|O_CREAT|O_TRUNC) ) < 0 )
	{
		return -1;
	}

	destseg = proc->memory;

	for(i=0; i<(proc->memsize+63)/64; i++)
	{
	   __movedata( destseg+i*64, 0, current_ds, (unsigned)bbuf, 1024);
	   if( write(fd, bbuf, 1024) < 0 )
	      printf("FAIL 7\n");
	}

	close(fd);
*/
}

/***************************************************************************/

seg_allocate(size)
{
   struct segment *nptr = calloc(sizeof(struct segment),1);

   if( nptr == 0 ) return 0;

   nptr->ref_count = 1;
   nptr->size = size;
   nptr->seg = __segalloc(size);
   if( nptr->seg == 0 ) { free(nptr); return 0; }

   return nptr;
}

seg_free(ptr)
struct segment * ptr;
{
   if(ptr == 0) return;
   if( --ptr->ref_count > 0 )
      return;
   __segfree(ptr->seg);
   free(ptr);
}
