# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

LSRC=syslibc.c
LOBJ=time.o abort.o wait.o waitpid.o killpg.o setpgrp.o sleep.o \
     usleep.o

LSRC3=syslib3.c
LOBJ3=__cstart3.o

LSRC0=syslib0.c
LOBJ0=__cstartup.o lseek.o getpid.o getppid.o getuid.o geteuid.o getgid.o \
      getegid.o dup2.o dup.o getpgrp.o times.o

ESRC=execve.c
E2OBJ=execl.o execv.o execle.o
EOBJ=execve.o $(E2OBJ)

DSRC=dirent.c
DOBJ=opendir.o closedir.o readdir.o

ifeq ($(LIB_CPU)-$(LIB_OS),i86-ELKS)
SYSCALLS=sh mksyscall
OBJ=$(LOBJ0) $(LOBJ) $(DOBJ) $(EOBJ) signal.o setjmp.o
DEP=mksyscall syscall.dat
endif

ifeq ($(LIB_CPU)-$(LIB_OS),i386-ELKS)
SYSCALLS=sh mksys386
OBJ=setjmp.o $(LOBJ3) $(LOBJ) $(E2OBJ) $(DOBJ)
DEP=mksys386 sys386.dat
endif

ifeq ($(SYSCALLS),)
SYSCALLS=true
OBJ=setjmp.o
DEP=
endif

all: $(DEP) $(OBJ)
	$(SYSCALLS)

libc.a:	$(DEP) $(OBJ)
	$(SYSCALLS) libc.a
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

clean:
	rm -f *.o libc.a syscall.c syscall.mak call_tab.v defn_tab.v

$(LOBJ): $(LSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(LSRC)

$(DOBJ): $(DSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(DSRC)

$(EOBJ): $(ESRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(ESRC)

$(LOBJ0): $(LSRC0)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(LSRC0)

$(LOBJ3): $(LSRC3)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(LSRC3)
