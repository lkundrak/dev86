# Copyright (C) 1996 Robert de Bath <robert@mayday.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

CC=bcc
CFLAGS=-O -ansi
LDFLAGS=

ifeq (Real_make,$(wildcard Real_make))
include Real_make
endif

ifneq ($(TOPDIR),)
starter:
	make -C ../libc/tests fetch_them
	make all
endif

not_from_here:
	@echo Do make tests from the top directory

clean:
	rm -f Real_make $(SRC) $(OBJ) $(EXE) $(LINK_FILES)
