# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

CSRC=cstartup.s
COBJ=cstartup.o 

ESRC=exec.c
EOBJ=execl.o execv.o execle.o execlp.o execvp.o

DSRC=dirent.c
DOBJ=opendir.o closedir.o readdir.o

HSRC=heap.c
HOBJ=__brk_addr.o brk.o sbrk.o

ifeq ($(LIB_CPU)-$(LIB_OS),g386-ELKS)
OBJ=$(COBJ) $(EOBJ) $(DOBJ) $(HOBJ)
SYSCALLS=syscalls

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

all: $(SYSCALLS) $(LIBC)($(OBJ))
	@$(RM) $(OBJ)

syscalls: syscall.mak
	$(MAKE) -f syscall.mak LIBC="$(LIBC)" CFLAGS="$(CFLAGS)"

syscall.mak: mksyscall syscall.dat
	sh mksyscall

$(LIBC)($(LOBJ)): $(LSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(COBJ)): $(CSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(DOBJ)): $(DSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(EOBJ)): $(ESRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(HOBJ)): $(HSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o
else
all:
	@:
endif

clean:
	rm -f *.o libc.a
	rm -f syscall.c syscall.mak

