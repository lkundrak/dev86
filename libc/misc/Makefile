# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

MSRC=aliases.c
MOBJ=labs.o abs.o raise.o bcopy.o bzero.o bcmp.o index.o rindex.o \
     remove.o creat.o

ESRC=atexit.c
EOBJ=on_exit.o atexit.o __do_exit.o

GOBJ=atoi.o atol.o ltoa.o ltostr.o \
     ctype.o ctypefn.o qsort.o bsearch.o rand.o lsearch.o getopt.o \
     itoa.o cputype.o strtol.o crypt.o

UOBJ=getenv.o putenv.o popen.o system.o setenv.o getcwd.o tmpnam.o

SSRC=syslib.c
SOBJ=time.o abort.o wait.o wait3.o waitpid.o killpg.o setpgrp.o sleep.o \
     usleep.o mkfifo.o

ifeq ($(LIB_OS),ELKS)
OBJ=$(MOBJ) $(EOBJ) $(GOBJ) $(UOBJ) $(SOBJ)
else
OBJ=$(MOBJ) $(EOBJ) $(GOBJ)
endif

# No ELKS strtod() until BCC does 16 bit FP...
ifneq ($(LIB_CPU),i86)
OBJ+=strtod.o atof.o
endif

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

all: $(LIBC)
	@$(RM) $(OBJ)

$(LIBC): $(LIBC)($(OBJ))

$(LIBC)($(MOBJ)): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(EOBJ)): $(ESRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(SOBJ)): $(SSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

clean:
	rm -f *.o libc.a

$(LIBC)(strtol.o): strtol.c
	$(CC) -c -ansi $(ARCH) $(CCFLAGS) $(DEFS) $*.c
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)(strtod.o): strtod.c
	$(CC) -c -ansi $(ARCH) $(CCFLAGS) $(DEFS) $*.c
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)(crypt.o): crypt.c
	$(CC) -c -ansi $(ARCH) $(CCFLAGS) $(DEFS) $*.c
	$(AR) $(ARFLAGS) $@ $*.o
