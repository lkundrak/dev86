# Copyright (C) 1996 Nat Friedman <ndf@aleph1.mit.edu>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

ifeq ($(LIB_OS),ELKS)
OBJ=utent.o
endif

all: $(OBJ)

libc.a: $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

%.o:%.c
ifeq ($(PLATFORM),i386-Linux)
	$(CC) $(CFLAGS) $< -c -o $@ $(WALL)
else
	$(CC) $(CFLAGS) $< -c -o $@ -ansi
endif

clean:
	rm -f *.o libc.a
