
CFLAGS=-O2 -D_POSIX_SOURCE
LDFLAGS=
DIRS=ld as

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
LIBDIR=$(PREFIX)/lib
MANDIR=$(PREFIX)/man/man1
SUF=86

INSTALL_OPT=-m 755
INSTALL_OPTS=-m 755 -s
MAN_OPTS    =-m 644

MFLAGS= "LIBS=$(LIBS)" \
	"CFLAGS=$(CFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" \
	"BINDIR=$(BINDIR)" \
	"LIBDIR=$(LIBDIR)" \
	"CC=$(CC)"

all:
	set -e ; for d in $(DIRS); do \
	  (cd $$d && $(MAKE) $(MFLAGS) $@;); \
	 done

install: all
	install $(INSTALL_OPTS) as/as86       $(BINDIR)/as$(SUF)
	install $(INSTALL_OPTS) ld/ld86       $(BINDIR)/ld$(SUF)
	install $(INSTALL_OPTS) ld/objdump86  $(BINDIR)/objdump$(SUF)
	install $(MAN_OPTS)     man/*.1       $(MANDIR)
	ln -sf                  objdump$(SUF) $(BINDIR)/nm$(SUF)
	ln -sf                  objdump$(SUF) $(BINDIR)/size$(SUF)

depend clean clobber:
	set -e ; for d in $(DIRS); do \
	  (cd $$d && $(MAKE) $(MFLAGS) $@;); \
	done

ungrab: ../as/as.c
	rm -rf as ld man
grab:
	cp -a ../as ../ld .
	rm -f ld/ar.h && cp -p ../libc/include/ar.h ld/ar.h
	mkdir man
	cp -p ../man/ld86.1 man
	cp -p ../man/as86.1 man
	touch -r ../Changes . as ld man

