# Copyright (C) 1996 Nat Friedman <ndf@aleph1.mit.edu>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

ifeq ($(LIB_CPU),g386)
CFLAGS+=$(WALL)
else
CFLAGS=$(CCFLAGS) $(LIBDEFS) -ansi
endif

GSRC=__getgrent.c grent.c getgrnam.c getgrgid.c fgetgrent.c initgroups.c \
	config-grp.h

ifeq ($(LIB_OS),ELKS)
GOBJ=__getgrent.o grent.o getgrnam.o getgrgid.o fgetgrent.o initgroups.o 
else
GOBJ=
endif

all: $(GOBJ)

%.o: %.c config-grp.h
	$(CC) $(CFLAGS) -o $@ $< -c

libc.a: $(GOBJ)
	ar r ../$(LIBC) $(GOBJ)
	@touch libc.a

test: test_grp.c libgrp.a
	$(CC) $(CFLAGS) test_grp.c -o test_grp -L -lgrp # -static

libgrp: libgrp.a

libgrp.a: $(GOBJ)
	ar r libgrp.a $(GOBJ)
	ranlib libgrp.a

clean:
	rm -f *.o libc.a libgrp.a
