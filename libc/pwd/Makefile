# Copyright (C) 1996 Nat Friedman <ndf@aleph1.mit.edu>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

ifeq ($(PLATFORM),i386-Linux)
CFLAGS+=$(WALL)
else # ifeq ($(PLATFORM),i86-ELKS)
CFLAGS=$(CCFLAGS) $(LIBDEFS) -ansi
endif

PSRC=__getpwent.c pwent.c getpwnam.c getpwuid.c putpwent.c getpw.c fgetpwent.c
POBJ=__getpwent.o pwent.o getpwnam.o getpwuid.o putpwent.o getpw.o fgetpwent.o

all: $(POBJ)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $< -c

libc.a: $(POBJ)
	ar r ../$(LIBC) $(POBJ)
	@touch libc.a

test: test_pwd.c libpwd.a
	$(CC) $(CFLAGS) test_pwd.c -o test_pwd -L. -lpwd

libpwd.a: $(POBJ)
	ar r libpwd.a $(POBJ)
	ranlib libpwd.a

libpwd: libpwd.a

clean:
	rm -f *.o libc.a libpwd.a




