#include <stdio.h>
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
	int i, fd;
	unsigned int ssize, size;
	unsigned destseg;
	struct elks_exec_hdr mh;

	if( ( fd = open(fname, O_RDONLY|O_BINARY) ) < 0 )
	{
		return -ENOEXEC;
	}

	/* Load the elks binary image and set it up in a suitable
	 * segment. Load CS and DS/SS according to image type.
	 */

	if( read(fd, &mh,sizeof(mh))!=sizeof(mh)
	 || mh.hlen!=EXEC_HEADER_SIZE
	 || (mh.type!=ELKS_COMBID&&mh.type!=ELKS_SPLITID)
         )
	{
	        close(fd);
		return -ENOEXEC;
	}

	size = ((mh.chmem+0xF) >> 4); 

	if(mh.type==ELKS_COMBID)
		ssize = mh.tseg+mh.dseg;
	else
	{
		ssize = mh.tseg;
		size += ((mh.tseg+0xF) >> 4); 
	}

   	destseg = __segalloc(size);	/* Allocate paragraphs */
   	if( destseg == 0 )
	{
	   close(fd);
	   return -ENOMEM;
        }

#ifdef DEBUGGER
	fprintf(stderr,"Executable - %s. tseg=0x%04lx dseg=0x%04lx bss=0x%04lx chmem=0x%04lx @ %04x\n",
		mh.type==ELKS_COMBID?"(impure)":"Sep I/D",
		mh.tseg,mh.dseg,mh.bseg,mh.chmem,destseg);
#endif

	proc->memory = destseg;
     	proc->memsize = size;
	proc->cs = destseg;

	for(size=0; size<ssize; )
	{
		i = sizeof(bbuf); if( i>ssize-size ) i=ssize-size;

		if(read(fd,bbuf,i)!=i)
		{
	           close(fd);
		   __segfree(destseg);
		   return -ENOEXEC;
		}
		
	   	__movedata(current_ds, (unsigned)bbuf, destseg, size, i);
	   	size += i;
	}

	if(mh.type==ELKS_SPLITID)
	{
		ssize = mh.dseg;

		destseg += ((mh.tseg+0xF) >> 4); 
		for(size=0; size<ssize; )
		{
			i = sizeof(bbuf); if( i>ssize-size ) i=ssize-size;

			if(read(fd,bbuf,i)!=i)
			{
	                    close(fd);
		            __segfree(destseg);
			    return -ENOEXEC;
			}
		
	   		__movedata(current_ds, (int)bbuf, destseg, size, i);
	   		size += i;
		}
	}
	close(fd);

	/* Zap the bss */
	ssize = size + mh.bseg;
	memset(bbuf, '\0', 1024);
	while(ssize > size)
	{
		i = sizeof(bbuf); if( i>ssize-size ) i=ssize-size;
	  	__movedata(current_ds, (int)bbuf, destseg, size, i);
   		size += i;
	}
	/*
	 *	Load the VM86 registers
	 */
	 
	/* proc->cs= Done above; */

	proc->ds=destseg;
	proc->ss=destseg;   /* Same as DS */

	proc->es=0;		/* This will be preserved */
	proc->sp=mh.chmem; 	/* Args stacked later */
	proc->pc=0;		/* Run from 0 */
	
	/*
	 *	Loaded
	 */

	return 0;
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
	printf("Dumping core\n");

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
}

