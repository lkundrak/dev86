# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

# Support for integer arithmetic 
ifeq ($(LIB_CPU),i86)
IOBJ=__idiv.o __idivu.o __imod.o __imodu.o __imul.o __isl.o __isr.o __isru.o
ISRC=bcc_int.c
endif

# Support for integer arithmetic when compiling for the i386
ifeq ($(LIB_CPU),i386)
ISRC=bcc_i386.c
IOBJ=__idiv.o __idivu.o __imod.o __imodu.o __imul.o __isl.o __isr.o __isru.o \
     __divsi3.o
endif

# Support for long arithmetic on little-endian (normal) longs 
LSRC=bcc_long.c
LOBJ=__laddl.o __landl.o __lcmpl.o __lcoml.o __ldecl.o __ldivl.o __ldivul.o \
     __leorl.o __lincl.o __lmodl.o __lmodul.o __lmull.o __lnegl.o __lorl.o \
     __lsll.o __lsrl.o __lsrul.o __lsubl.o __ltstl.o

# Memory allocation primitives
AOBJ=errno.o __brk_addr.o sbrk.o brk.o
ASRC=heap.c

# Support for long arithmetic on big-endian (words-swapped) longs 
RSRC=bcc_bsw.c
ROBJ=__laddb.o __landb.o __lcmpb.o __lcomb.o __ldecb.o __ldivb.o __ldivub.o \
     __leorb.o __lincb.o __lmodb.o __lmodub.o __lmulb.o __lnegb.o __lorb.o \
     __lslb.o __lsrb.o __lsrub.o __lsubb.o __ltstb.o

# Miscellaneous I/O and far access junk 
PSRC=bcc_io.c
POBJ=__inport.o __inportb.o __outport.o __outportb.o __peekb.o __peekw.o \
     __pokeb.o __pokew.o

ifeq ($(LIB_CPU),i86)
OBJ=__ldivmod.o $(IOBJ) $(LOBJ) $(AOBJ)
endif
ifeq ($(LIB_CPU),i386)
OBJ=$(IOBJ) $(AOBJ)
endif

OLDOBJ=$(ROBJ) $(POBJ)

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

ifneq ($(OBJ),)

all: $(LIBC)
	@$(RM) $(OBJ)

$(LIBC): $(LIBC)($(OBJ))

$(LIBC)($(IOBJ)): $(ISRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(LOBJ)): $(LSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(AOBJ)): $(ASRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(ROBJ)): $(RSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(POBJ)): $(PSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o
else
all:
	@:
endif

clean:
	rm -f *.o libc.a
