# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

all:
	@:

transfer:
	-@rm -f ../include/linuxmt
	ln -s ../kinclude/linuxmt ../include
	@touch Used

# This is for use once linuxmt's syscall interface really gets working.
# beware the arch directory must be removed when you do this.
real_transfer:
	-@rm -f ../include/linuxmt
	cd ../include ; ln -s ../../linuxmt/include/linuxmt .

clean:
	-@rm -f ../include/linuxmt
	-@rm -f Used
