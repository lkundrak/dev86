
export TOPDIR := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)

include Make.defs

PARTS=    ld as unproto bcc
LIBS2=    libbsd
LIBS=     libc $(LIBS2)
EXTRAS=   man dis88 doselks
TESTDIRS= tests
DISTFILES=Makefile Make.defs README Changes Libc_version Uninstall
DISTDIRS= $(LIBS2) elksemu $(TESTDIRS) $(EXTRAS)
DOSBITS=  mkcompile compile.bat later.c

default: dummy
	@echo "You have to do 'make install' as root"
	@echo Or:
	@echo
	@echo '$ make bcc'
	@echo '$ su -c "make install-bcc"'
	@echo '$ make library'
	@echo '$ su -c "make install-lib"'
	@echo '$ make elksemu'
	@echo '$ su -c "make install-emu"'
	@echo '$ su -c "make install-man"'
	@echo
	@echo 'Other libraries are built with:'
	@echo '$ su -c "make install-lib2"'
	@echo
	@echo "Or do 'make install-all' for _everything_"

dummy:
	@if [ -f .runme ] ; then sh .runme ; rm .runme ; fi

install: install-bcc install-lib install-emu install-man

# Do _everything_!
install-all:
	make realclean
	make config
	make install-bcc
	make install-man
	make install-lib-fast
	make install-lib-dos
	make install-lib-bios
	make install-lib-386
	make install-lib
	make install-emu
	make -C dis88 install
	make realclean

config:
	make -C libc config

all: bcc library elksemu tests extras

bcc: dummy
	@for i in $(PARTS) ; do make -C $$i || exit 1; done

realclean:
	@for i in $(PARTS) libc $(DISTDIRS) ; do \
	   if grep -q '^realclean' $$i/Makefile ; then \
	   make -C $$i realclean ; else \
	   make -C $$i clean ; fi ; done

clean:
	@for i in $(PARTS) libc $(DISTDIRS) ; do \
	   make -C $$i clean || exit 1; done

tests: dummy
	@test -f $(BINDIR)/bcc || \
	( echo 'Must do "make install-bcc" first' && exit 1 )
	@test -f $(LIBDIR)/i86/crt0.o || \
	( echo 'Must do "make install-lib" first' && exit 1 )
	@for i in $(TESTDIRS) ; do make -C $$i || exit 1; done

library: dummy
	@test -f $(BINDIR)/bcc || \
	( echo 'Must do "make install-bcc" first' && exit 1 )
	make -C libc PLATFORM=i86-ELKS

elksemu: dummy
	@test -f libc/syscall/call_tab.v || \
	( echo 'Must do "make library" first' && exit 1 )
	make -C elksemu

extras: dummy
	@for i in $(EXTRAS) ; do make -C $$i || exit 1; done
	
install-bcc: dummy
	@for i in $(PARTS) ; do make -C $$i install || exit 1; done

install-lib: dummy
	@test -f $(BINDIR)/bcc || \
	( echo 'Must do "make install-bcc" first' && exit 1 )
	@for i in $(LIBS) ; do \
	 make -C $$i PLATFORM=i86-ELKS install || exit 1 ; \
	 done

install-lib2: install-lib-bios install-lib-dos install-lib-fast install-lib-386

install-lib-bios: dummy
	@test -f $(BINDIR)/bcc || \
	( echo 'Must do "make install-bcc" first' && exit 1 )
	make -C libc PLATFORM=i86-BIOS install

install-lib-dos: dummy
	@test -f $(BINDIR)/bcc || \
	( echo 'Must do "make install-bcc" first' && exit 1 )
	make -C libc PLATFORM=i86-DOS install

install-lib-fast: dummy
	@test -f $(BINDIR)/bcc || \
	( echo 'Must do "make install-bcc" first' && exit 1 )
	make -C libc PLATFORM=i86-FAST install

install-lib-386: dummy
	@test -f $(BINDIR)/bcc || \
	( echo 'Must do "make install-bcc" first' && exit 1 )
	make -C libc PLATFORM=i386-BCC install

install-emu: dummy
	@test -f libc/syscall/call_tab.v || \
	( echo 'Must do "make library" first' && exit 1 )
	make -C elksemu install

install-man: dummy
	make -C man install

install-extras: dummy
	@for i in $(EXTRAS) ; do make -C $$i install || exit 1; done
	
distribution:
	sh Build_dist
