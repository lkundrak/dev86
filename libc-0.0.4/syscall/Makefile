# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

LSRC=syslibc.c
LOBJ=__cstartup.o time.o lseek.o getpid.o getppid.o \
     getuid.o geteuid.o getgid.o getegid.o \
     dup2.o dup.o abort.o wait.o waitpid.o sleep.o

ESRC=execve.c
EOBJ=execve.o execl.o execv.o execle.o

DSRC=dirent.c
DOBJ=opendir.o closedir.o readdir.o

OBJ=$(LOBJ) $(DOBJ) $(EOBJ) signal.o setjmp.o execve.o

all: mksyscall syscall.dat $(OBJ)
	sh mksyscall

clean:
	sh mksyscall clean
	rm -f libc.a syscall.c syscall.mak call_tab.v defn_tab.v $(OBJ)

libdos.a: setjmp.o
	ar r ../libc.a setjmp.o
	@touch libc.a

libc.a:	mksyscall syscall.dat $(OBJ)
	sh mksyscall libc.a
	ar r ../libc.a $(OBJ)
	@touch libc.a

$(LOBJ): $(LSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(LSRC)

$(DOBJ): $(DSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(DSRC)

$(EOBJ): $(ESRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(ESRC)

