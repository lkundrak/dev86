# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

MSRC=aliases.c
MOBJ=abs.o raise.o bcopy.o bzero.o bcmp.o index.o rindex.o remove.o creat.o

ESRC=atexit.c
EOBJ=on_exit.o atexit.o __do_exit.o

GOBJ=atoi.o atol.o ltoa.o ltostr.o \
     ctype.o qsort.o bsearch.o rand.o lsearch.o getopt.o \
     itoa.o cputype.o strtol.o crypt.o

UOBJ=getenv.o putenv.o popen.o system.o setenv.o


ifeq ($(LIB_OS),ELKS)
OBJ=$(MOBJ) $(EOBJ) $(GOBJ) $(UOBJ)
else
OBJ=$(MOBJ) $(EOBJ) $(GOBJ)
endif

# No ELKS strtod() until BCC does 16 bit FP...
ifneq ($(LIB_CPU),i86)
OBJ+=strtod.o
endif

all: $(OBJ)

libc.a: $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

clean:
	rm -f *.o libc.a

$(MOBJ): $(MSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(MSRC)

$(EOBJ): $(ESRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(ESRC)

crypt.o: crypt.c
ifeq ($(LIB_CPU),g386)
	$(CC) $(CFLAGS) $< -c -o $@ $(WALL)
else
	$(CC) $(CFLAGS) $< -c -o $@ -ansi
endif
	
strto%.o: strto%.c
ifeq ($(LIB_CPU),g386)
	$(CC) $(CFLAGS) $< -c -o $@ $(WALL)
else
	$(CC) $(CFLAGS) $< -c -o $@ -ansi
endif
