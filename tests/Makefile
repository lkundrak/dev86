# Copyright (C) 1996 Robert de Bath <robert@mayday.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

BCC=bcc
CC=$(BCC)
CFLAGS=-O

SRC=env.c ft.c hd.c size.c sync.c compr.c ucomp.c ouch.c lines.c \
    wc.c line2.c rand.c grab.c
OBJ=
EXE=env ft hd size sync compr ucomp ouch lines wc line2 rand grab

LINK_FILES=cat chgrp chmod chown cp install ln mkdir mkfifo mknod mv rm

all: $(EXE)

install links:
	for i in $(LINK_FILES) ; do ln -s ft $$i ; done

no_links:
	rm -f $(LINK_FILES)

clean realclean:
	rm -f $(OBJ) $(EXE) $(LINK_FILES)

