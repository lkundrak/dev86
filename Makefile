# Copyright (C) 1997 Robert de Bath <robert@mayday.cix.co.uk>
# This file is part of the Linux-8086 Development environment and is
# distributed under the GNU General Public License.

VERSION=0.16.17

TARGETS=install clean other \
    bcc86 unproto copt as86 ld86 elksemu \
    install-all install-bcc install-emu install-lib \
    install-lib2 install-ln install-man install-other \
    all-libs alt-libs library lib-386 lib-bsd lib-dos lib-fast lib-stand \
    config tests dis88 doselks bootblocks ld86r

ELKSSRC= /usr/src/elks
PREFIX=  /usr
BINDIR=	 $(PREFIX)/bin
LIBDIR=  $(PREFIX)/lib/bcc
INCLDIR= $(PREFIX)/lib/bcc
ASLDDIR= $(BINDIR)
MANDIR=	 $(PREFIX)/man
CFLAGS=  -O

# Some makes take the last of a list as the default ...
all: make.fil
	PATH="`pwd`/bin:$$PATH" $(MAKE) -f make.fil VERSION=$(VERSION) TOPDIR=`pwd` $@

$(TARGETS): make.fil
	PATH="`pwd`/bin:$$PATH" $(MAKE) -f make.fil VERSION=$(VERSION) TOPDIR=`pwd` $@

$(TARGETS):

ld: ld86
as: as86

realclean:
	-[ ! -f make.fil ] || $(MAKE) -f make.fil VERSION=$(VERSION) TOPDIR=`pwd` $@
	-rm -f make.fil ifdef ifdef.o

make.fil: ifdef makefile.in
	./ifdef -MU makefile.in >tmp.mak
	echo > tmp.sed
	[ "$(BINDIR)" != "//bin" ] || echo >> tmp.sed "s:%BINDIR%:/bin:"
	[ "$(LIBDIR)" != "//lib/bcc" ] || echo >> tmp.sed "s:%LIBDIR%:/lib:"
	[ "$(INCLDIR)" != "//lib/bcc" ] || echo >> tmp.sed "s:%INCLDIR%:/usr:"
	[ "$(ASLDDIR)" != "//bin" ] || echo >> tmp.sed "s:%ASLDDIR%:/bin:"
	[ "$(MANDIR)" != "//man" ] || echo >> tmp.sed "s:%MANDIR%:/usr/man:"
	echo >> tmp.sed "s:%PREFIX%:$(PREFIX):"
	echo >> tmp.sed "s:%BINDIR%:$(BINDIR):"
	echo >> tmp.sed "s:%INCLDIR%:$(INCLDIR):"
	echo >> tmp.sed "s:%LIBDIR%:$(LIBDIR):"
	echo >> tmp.sed "s:%ASLDDIR%:$(ASLDDIR):"
	echo >> tmp.sed "s:%MANDIR%:$(MANDIR):"
	echo >> tmp.sed "s:%ELKSSRC%:$(ELKSSRC):"
	echo >> tmp.sed "s:%CC%:$(CC):"
	echo >> tmp.sed "s:%CFLAGS%:$(CFLAGS):"
	echo >> tmp.sed "s:%LDFLAGS%:$(LDFLAGS):"
	sed -f tmp.sed < tmp.mak > make.tmp
	mv -f make.tmp make.fil
	@rm -f tmp.mak tmp.sed

ifdef: ifdef.o
	$(CC) $(IFDEFARCH) $(LDFLAGS) -o ifdef ifdef.o

ifdef.o: ifdef.c
	$(CC) $(IFDEFARCH) $(CFLAGS) $(IFDEFFLAGS) -c ifdef.c

uninstall:
	@echo 'Sorry, no go; it was just wrong.'
	false

distribution:
	@[ `id -u` -eq 0 ] || fakeroot -- sh ./Mk_dist $(VERSION)
	@[ `id -u` -ne 0 ] || sh ./Mk_dist $(VERSION)
