# libbsd	A collection of library modules similar or equal to their
#		BSD UNIX equivalents, to make the job of porting BSD pro-
#		grams to LINUX easier.
#
# Author:	Rick Sladkey, <jrs@world.std.com>
# Ported to linux-86 (by removing select.c):	Dick Porter <dick@cymru.net>
#
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=$(TOPDIR)
include Make.defs

LIBBSD=libbsd.a
TARGETS=$(LIBBSD)
SRCS= daemon.c logwtmp.c logout.c pty.c login_tty.c \
	sgtty.c cfsetspeed.c revoke.c setpassent.c \
	checkrhosts.c
OBJS=$(SRCS:.c=.o)

TXT=COPYING Changes Contributors MAGIC \
    Make.defs Makefile New_subdir Pre_main README TODO KERNEL

all: $(TARGETS)

install: all
	install -d $(BCCHOME)
	install -d $(LIBDIR)/i86
	install -m 644 $(LIBBSD) $(LIBDIR)/i86

tests: dummy
	make -C tests

$(LIBBSD): $(OBJS)
	$(AR) rc $(LIBBSD) $(OBJS)

realclean: clean
clean: dummy
	rm -f *.o
	rm -f $(LIBBSD)

dummy:
