# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

OBJ=

all: $(OBJ)

libc.a: 

transfer:
	-@rm -f ../include/linuxmt
	ln -s ../kinclude/linuxmt ../include

# This is for use once linuxmt's syscall interface really gets working.
real_transfer:
	-@rm -f ../include/linuxmt
	cd ../include ; ln -s ../../linuxmt/include/linuxmt .

clean:
