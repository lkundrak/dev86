
export TOPDIR := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)

include Make.defs

PARTS=    ld as unproto bcc
TESTDIRS= tests dis88
DISTFILES=Makefile Make.defs README README.ash Libc_version
DISTDIRS= elksemu $(TESTDIRS)

default: dummy
	@echo You have to do make install as root
	@echo Or:
	@echo
	@echo 'make bcc'
	@echo 'su -c "make install-bcc"'
	@echo 'make library'
	@echo 'su -c "make install-lib"'
	@echo 'make elksemu'
	@echo 'su -c "make install-emu"'
	@echo 'make tests'
	@echo

dummy:
	@if [ -f .runme ] ; then sh .runme ; rm .runme ; fi

install: install-bcc install-lib install-emu tests

bcc: dummy
	@for i in $(PARTS) ; do make -C $$i || exit 1; done

clean: clean_rest
	make -C libc realclean

clean_rest: dummy
	@for i in $(PARTS) ; do make -C $$i clean || exit 1; done
	@for i in $(DISTDIRS) ; do make -C $$i clean || exit 1; done

tests: dummy
	@test -f $(BINDIR)/bcc -a -f $(LIBDIR)/ld86 || \
	( echo 'Must do "make install-bcc" first' && exit 1 )
	@test -f $(LIBDIR)/i86/crt0.o || \
	( echo 'Must do "make install-lib" first' && exit 1 )
	@for i in $(TESTDIRS) ; do make -C $$i || exit 1; done

library: dummy
	@test -f $(BINDIR)/bcc -a -f $(LIBDIR)/ld86 || \
	( echo 'Must do "make install-bcc" first' && exit 1 )
	make -C libc

elksemu: dummy
	@test -f libc/syscall/call_tab.v || \
	( echo 'Must do "make library" first' && exit 1 )
	make -C elksemu

install-bcc: dummy
	@for i in $(PARTS) ; do make -C $$i install || exit 1; done

install-lib: dummy
	@test -f $(BINDIR)/bcc -a -f $(LIBDIR)/ld86 || \
	( echo 'Must do "make install-bcc" first' && exit 1 )
	make -C libc install

install-lib2: dummy
	@test -f $(BINDIR)/bcc -a -f $(LIBDIR)/ld86 || \
	( echo 'Must do "make install-bcc" first' && exit 1 )
	make -s -C libc clean
	make -s -C libc PLATFORM=i86-FAST install
	make -s -C libc clean
	make -s -C libc PLATFORM=i86-DOS install
	make -s -C libc clean

install-emu: dummy
	@test -f libc/syscall/call_tab.v || \
	( echo 'Must do "make library" first' && exit 1 )
	make -C elksemu install

distribution: clean_rest
	make -C libc dist_ver
	tar  cf /tmp/Development.tar $(DISTFILES) $(PARTS) $(DISTDIRS)
	rm -rf /tmp/linux-86
	mkdir /tmp/linux-86
	ln -s `pwd`/libc-8086-`cat Libc_version`.tar.gz /tmp/libc-8086.tgz
	cd /tmp/linux-86 ; tar xzf ../libc-8086.tgz ; rm ../libc-8086.tgz
	mv /tmp/linux-86/libc-`cat Libc_version` /tmp/linux-86/libc
	cd /tmp/linux-86 ; tar xf ../Development.tar ; rm ../Development.tar
	cd /tmp; tar czf Development.tar.gz linux-86 ; rm -rf linux-86
	mv /tmp/Development.tar.gz /tmp/Dev86-`cat Libc_version`.tar.gz
	rm -rf libc-`cat Libc_version` Libc_version

