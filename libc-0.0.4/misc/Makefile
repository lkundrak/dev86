# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

MSRC=aliases.c
MOBJ=abs.o raise.o bcopy.o bzero.o bcmp.o index.o rindex.o remove.o creat.o

ESRC=atexit.c
EOBJ=on_exit.o atexit.o __do_exit.o

OBJ=$(MOBJ) $(EOBJ) \
    atoi.o atol.o ltoa.o ltostr.o \
    ctype.o qsort.o bsearch.o rand.o lsearch.o getopt.o \
    getenv.o putenv.o itoa.o cputype.o \
    strtol.o popen.o system.o

# No ELKS strtod() until BCC does 16 bit FP...
ifeq ($(PLATFORM),i386-Linux)
OBJ+=strtod.o
endif

all: $(OBJ)

$(LIBC): $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

clean:
	rm -f $(OBJ) libc.a

$(MOBJ): $(MSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(MSRC)

$(EOBJ): $(ESRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(ESRC)

strto%.o: strto%.c
ifeq ($(PLATFORM),i386-Linux)
	$(CC) $(CFLAGS) $< -c -o $@ $(WALL)
else
	$(CC) $(CFLAGS) $< -c -o $@ -ansi
endif
