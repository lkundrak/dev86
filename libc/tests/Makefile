# Copyright (C) 1996 Robert de Bath <robert@mayday.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs
CFLAGS=$(CCFLAGS) -ansi

default: all

libc.a:
	@echo -n

include Real_make

fetch_them:
	cp -p $(SRC) Real_make $(TOPDIR)/tests/.

clean:
	rm -f $(OBJ) $(EXE) $(LINK_FILES)
