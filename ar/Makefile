# Copyright (c) 1999 Greg Haerr <greg@censoft.com>
# This file is part of the Linux-8086 Development environment and is
# distributed under the GNU General Public License.

LIBDIR	=/usr/bin
CFLAGS	=-O
LDFLAGS	=
DEFS	=
OBJS= ar.o

all: ar86

ar86: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

install: ar86
	install -d $(LIBDIR)
	install -m 755 ar86 $(LIBDIR)

clean realclean clobber:
	rm -f *.o ar86 ar.h rel_aout.h

$(OBJS): ar.h rel_aout.h

ar.h:
	[ -f ar.h ] || \
	{ rm -f ar.h ; ln -s ../libc/include/ar.h . ; } || \
	ln ../libc/include/ar.h .

rel_aout.h:
	[ -f rel_aout.h ] || \
	{ rm -f rel_aout.h ; ln -s ../ld/rel_aout.h . ; } || \
	ln ../ld/rel_aout.h .

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@
