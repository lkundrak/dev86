
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/vm86.h>
#include "elks.h" 

static int elks_sigtrap= -1;

void sig_trap(int signo)
{
   elks_cpu.regs.esp -= 2;
   ELKS_POKE(unsigned short, elks_cpu.regs.esp, signo);
   elks_cpu.regs.esp -= 2;
   ELKS_POKE(unsigned short, elks_cpu.regs.esp, elks_cpu.regs.eip);
   elks_cpu.regs.eip = elks_sigtrap;
}

int elks_signal(int bx,int cx,int dx,int di,int si)
{
   int rv;
   if( bx < 0 || bx >= NSIG ) { errno = EINVAL; return -1; }
   if( cx == 0 )      rv = (signal(bx, SIG_DFL) == SIG_ERR);
   else if( cx == 1 ) rv = (signal(bx, SIG_IGN) == SIG_ERR);
   else 
   {
      elks_sigtrap = cx;
      rv = (signal(bx, sig_trap) == SIG_ERR);
   }

   return -rv;
}
