# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

LSRC=cstartup.c
LOBJ=__cstart3.o

ESRC=exec.c
EOBJ=execl.o execv.o execle.o execlp.o execvp.o

DSRC=dirent.c
DOBJ=opendir.o closedir.o readdir.o

ifeq ($(LIB_CPU)-$(LIB_OS),i386-ELKS)
OBJ=$(LOBJ3) $(LOBJ) $(EOBJ) $(DOBJ) setjmp3.o
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

$(LIBC)($(DOBJ)): $(DSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(EOBJ)): $(ESRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o
else
all:
	@:
endif

clean:
	rm -f *.o libc.a
	rm -f syscall.c syscall.mak

