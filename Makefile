# Copyright (C) 1997 Robert de Bath <robert@mayday.cix.co.uk>
# This file is part of the Linux-8086 Development environment and is
# distributed under the GNU General Public License.

TARGETS= \
    clean bcc unproto copt as86 ld86 elksemu \
    install install-all install-bcc install-emu install-lib \
    install-lib2 install-ln install-man install-other \
    all-libs alt-libs library lib-386 lib-bsd lib-dos lib-fast lib-stand \
    config other tests dis88 doselks bootblocks ld86r

PREFIX=	 /usr/bcc
BINDIR=	 /usr/bin
LIBDIR=  $(PREFIX)/lib/bcc
CFLAGS=  -O

# Some makes take the last of a list as the default ...
all: make.fil
	PATH="`pwd`/bin:$$PATH" $(MAKE) -f make.fil TOPDIR=`pwd` $@

$(TARGETS): make.fil
	PATH="`pwd`/bin:$$PATH" $(MAKE) -f make.fil TOPDIR=`pwd` $@

$(TARGETS): phony
phony:

ld: ld86
as: as86

realclean:
	-[ ! -f make.fil ] || $(MAKE) -f make.fil TOPDIR=`pwd` $@
	-rm -f make.fil ifdef ifdef.o

make.fil: ifdef makefile.in
	./ifdef -MU makefile.in >tmp.mak
	sed -e "s:%PREFIX%:$(PREFIX):" \
	    -e "s:%BINDIR%:$(BINDIR):" \
	    -e "s:%LIBDIR%:$(LIBDIR):" \
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

Uninstall: phony
	@# CHECK FROM HERE
	@make -n Uninstall
	@echo 'Are you really sure... have you checked this... ^C to interrupt'
	@read line
	rm -rf /usr/bcc
	rm -f $(BINDIR)/bcc $(BINDIR)/as86_encap $(BINDIR)/dis86
	rm -f $(BINDIR)/as86 $(BINDIR)/ld86
	rm -f /lib/elksemu
	rm -f /usr/lib/liberror.txt
	rm -f /usr/man/man1/elks.1* /usr/man/man1/elksemu.1*
	rm -f /usr/man/man1/dis86.1* /usr/man/man1/bcc.1*
	rm -f /usr/man/man1/as86.1* /usr/man/man1/ld86.1*
	@# TO HERE

distribution:
	@rm -f /tmp/linux-86 || true
	@[ ! -f Copy_dist ] || sh Copy_dist
	mkdir -p /tmp/Dist
	[ -d /tmp/linux-86 ] || ln -s `pwd` /tmp/linux-86
	cd /tmp 							&&\
	$(MAKE) -C linux-86 realclean					&&\
	$(MAKE) -C linux-86/libc Libc_version				&&\
	VER=`cat linux-86/Libc_version`					&&\
	tar cf Dist/Dev86src-$$VER.tar linux-86/*			&&\
	ln -s linux-86/as as86						&&\
	cp -p linux-86/man/as86.1 as86/as86.1				&&\
	cp -p linux-86/COPYING as86/COPYING				&&\
	tar cf Dist/as86-$$VER.tar `find as86/* -prune -type f`		&&\
	rm as86/as86.1 as86						&&\
	ln -s linux-86/libc libc-$$VER 					&&\
	tar cf Dist/libc-8086-$$VER.tar libc-$$VER/*			&&\
	rm libc-$$VER							&&\
	$(MAKE) -C /tmp/linux-86 install 				\
		ARFLAGS=q DIST=/tmp/linux-86-dist ELKSSRC=/dev/null	&&\
	$(MAKE) -C /tmp/linux-86 other					&&\
	tar cf /tmp/Dist/Dev86bin-$$VER.tar -C /tmp/linux-86-dist .	&&\
	rm -f /tmp/Dist/Dev86clb-$$VER.zip Bcc				&&\
	ln -s /tmp/linux-86 Bcc						&&\
	zip -9rpk /tmp/Dist/Dev86clb-$$VER.zip \
		Bcc/lib/crt0.o Bcc/lib/libc.a Bcc/lib/libbsd.a \
		Bcc/lib/libdos.a Bcc/lib/libc_f.a Bcc/lib/libc_s.a \
		Bcc/lib/i386/crt0.o Bcc/lib/i386/libc.a			&&\
	rm Bcc

	gzip -v9f /tmp/Dist/*.tar
	@rm /tmp/linux-86 || true
