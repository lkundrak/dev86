
export TOPDIR := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)

include Make.defs

PARTS=    ld as unproto bcc
TESTDIRS= tests dis88
DISTFILES=Makefile Make.defs README README.ash Libc_version
DISTDIRS= elksemu $(TESTDIRS)

default:
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

library: link_libc
	@test -f $(BINDIR)/bcc -a -f $(LIBDIR)/ld86 || \
	( echo 'Must do "make install-bcc" first' && exit 1 )
	make -C libc

elksemu: dummy
	@test -f libc/syscall/call_tab.v || \
	( echo 'Must do "make library" first' && exit 1 )
	make -C elksemu

install-bcc: dummy
	@for i in $(PARTS) ; do make -C $$i install || exit 1; done

install-lib: link_libc
	@test -f $(BINDIR)/bcc -a -f $(LIBDIR)/ld86 || \
	( echo 'Must do "make install-bcc" first' && exit 1 )
	make -C libc install

install-emu: dummy
	@test -f libc/syscall/call_tab.v || \
	( echo 'Must do "make library" first' && exit 1 )
	make -C elksemu install

link_libc: dummy
	@if [ -f Libc_version ] ; then \
	   rm -f libc ; ln -fs libc-`cat Libc_version` libc ; fi

distribution: clean_rest
	-rm -f libc
	make -C libc dist_ver
	tar xzf libc-8086-`cat Libc_version`.tar.gz
	tar  cf /tmp/Development.tar $(DISTFILES) $(PARTS) $(DISTDIRS) \
		libc-`cat Libc_version`
	rm -rf libc-`cat Libc_version` Libc_version
	rm -rf /tmp/linux-86
	mkdir /tmp/linux-86
	(cd /tmp/linux-86 ; tar xf ../Development.tar ; rm ../Development.tar)
	(cd /tmp; tar czf Development.tar.gz linux-86 ; rm -rf linux-86)

dummy:
