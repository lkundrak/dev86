# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

LSRC=syslib0.c
LOBJ=__cstartup.o lseek.o getpid.o getppid.o getuid.o geteuid.o getgid.o \
      getegid.o dup2.o getpgrp.o times.o

ESRC=exec.c
EOBJ=execve.o execl.o execv.o execle.o execlp.o execvp.o

DSRC=dirent.c
DOBJ=opendir.o closedir.o readdir.o

ifeq ($(LIB_CPU)-$(LIB_OS),i86-ELKS)
OBJ=$(LOBJ) $(DOBJ) $(EOBJ) signal.o setjmp.o
SYSCALLS=call_i86
endif

ifeq ($(SYSCALLS),)
OBJ=setjmp.o
endif

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

all: $(SYSCALLS) $(LIBC)
	@$(RM) $(OBJ)

call_i86: syscall.mak
	$(MAKE) -f syscall.mak LIBC="$(LIBC)" CFLAGS="$(CFLAGS)"

syscall.mak: mksyscall syscall.dat
	sh mksyscall

syscall.dat:
	@touch syscall.dat

$(LIBC): $(LIBC)($(OBJ))

$(LIBC)($(LOBJ)): $(LSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(DOBJ)): $(DSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(EOBJ)): $(ESRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

clean:
	rm -f *.o libc.a
	rm -f syscall.c syscall.mak syscall.dat
	rm -f call_tab.v defn_tab.v
