# Copyright (C) 1996 Robert de Bath <robert@debath.thenet.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

ifeq ($(LIB_OS),ELKS)
OBJ=error.o sys_errlist.o perror.o sys_siglist.o __assert.o
endif

all: $(OBJ)

libc.a: $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

clean:
	rm -f *.o libc.a
