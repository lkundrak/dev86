# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

all:
	@:

transfer:
	-@rm -f ../include/linuxmt ../include/arch
	ln -s ../kinclude/linuxmt ../include
	ln -s ../kinclude/arch ../include

clean:
	-rm -f ../include/linuxmt ../include/arch
