# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

ifeq ($(LIB_OS),ELKS)
OBJ=termcap.o tparam.o
endif

all: $(OBJ)

libc.a: $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

clean:
	rm -f *.o libc.a

$(SOBJ): $(SSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(SSRC)

