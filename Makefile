# Copyright (C) 1997 Robert de Bath <robert@mayday.cix.co.uk>
# This file is part of the Linux-8086 Development environment and is
# distributed under the GNU General Public License.

TARGETS= \
    clean bcc unproto copt as86 ld86 elksemu \
    install install-all install-bcc install-emu install-lib \
    install-lib2 install-ln install-man install-other \
    all-libs alt-libs library lib-386 lib-bsd lib-dos lib-fast lib-stand \
    config other tests dis88 doselks bootblocks ld86r

ELKSSRC= /usr/src/elks
PREFIX=  /usr
LIBPRE=  $(PREFIX)/bcc
BINDIR=	 $(PREFIX)/bin
LIBDIR=  $(LIBPRE)/lib/bcc
CFLAGS=  -O

# Some makes take the last of a list as the default ...
all: make.fil
	PATH="`pwd`/bin:$$PATH" $(MAKE) -f make.fil TOPDIR=`pwd` $@

$(TARGETS): make.fil
	PATH="`pwd`/bin:$$PATH" $(MAKE) -f make.fil TOPDIR=`pwd` $@

$(TARGETS):

ld: ld86
as: as86

realclean:
	-[ ! -f make.fil ] || $(MAKE) -f make.fil TOPDIR=`pwd` $@
	-rm -f make.fil ifdef ifdef.o

make.fil: ifdef makefile.in
	./ifdef -MU makefile.in >tmp.mak
	sed \
	    -e "s:%PREFIX%:$(PREFIX):" \
	    -e "s:%LIBPRE%:$(LIBPRE):" \
	    -e "s:%BINDIR%:$(BINDIR):" \
	    -e "s:%LIBDIR%:$(LIBDIR):" \
	    -e "s:%ELKSSRC%:$(ELKSSRC):" \
	    -e "s:%CC%:$(CC):" \
	    -e "s:%CFLAGS%:$(CFLAGS):" \
	    -e "s:%LDFLAGS%:$(LDFLAGS):" \
	       < tmp.mak > make.tmp
	mv -f make.tmp make.fil
	@rm -f tmp.mak

ifdef: ifdef.o
	$(CC) $(LDFLAGS) -o ifdef ifdef.o

ifdef.o: ifdef.c
	$(CC) $(CFLAGS) $(IFDEFFLAGS) -c ifdef.c

Uninstall:
	@# CHECK FROM HERE
	@make -n Uninstall
	@echo 'Are you really sure... have you checked this... ^C to interrupt'
	@read line
	rm -rf /usr/bcc
	rm -f $(BINDIR)/bcc $(BINDIR)/as86_encap $(BINDIR)/dis86
	rm -f $(BINDIR)/as86 $(BINDIR)/ld86
	rm -f $(BINDIR)/objdump86 $(BINDIR)/nm86 $(BINDIR)/size86
	rm -f /lib/elksemu
	rm -f /usr/lib/liberror.txt
	rm -f /usr/man/man1/elks.1* /usr/man/man1/elksemu.1*
	rm -f /usr/man/man1/dis86.1* /usr/man/man1/bcc.1*
	rm -f /usr/man/man1/as86.1* /usr/man/man1/ld86.1*
	@# TO HERE

distribution:
	@[ `id -u` -eq 0 ] || fakeroot -- sh ./Mk_dist
	@[ `id -u` -ne 0 ] || sh ./Mk_dist
