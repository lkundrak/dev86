# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=.
include $(TOP)/Make.defs

SRC=crt0.c
OBJ=crt0.o

TARGETS=$(OBJ) $(LIBC)
TXT=Makefile Make.defs README KERNEL COPYING Contributors MAGIC \
    New_subdir Pre_main Config_sh

all: .config.dir .config.otype $(TARGETS)

install: all install_incl
	install -d $(BCCHOME)
	install -d $(LIBDIR)/$(LIB_CPU)
	install -m 644 crt0.o $(LIBDIR)/$(LIB_CPU)
	install -m 644 $(LIBC) $(LIBDIR)/$(LIB_CPU)
	-install -d $(DIST)/usr/lib
	-install -m 644 error/liberror.txt $(DIST)/usr/lib/liberror.txt

# I've changed this so it'll be easier to make a binary dist.
old_install_incl:
	rm -rf $(BCCHOME)/include
	ln -s $(TOPDIR)/libc/include $(BCCHOME)/include

install_incl:
	rm -rf $(BCCHOME)/include
	cp -pr include $(BCCHOME)/include
	if [ -f kinclude/Used ] ; \
	then cp -pr kinclude/arch $(BCCHOME)/include/arch ; \
	else rm -rf $(BCCHOME)/include/linuxmt ; \
	     ln -s $(ELKSSRC)/include/linuxmt $(BCCHOME)/include ; \
	fi
	-chown -R root:root   $(BCCHOME)/include 2>/dev/null
	-chmod -R u=rwX,og=rX $(BCCHOME)/include

tests: dummy
	$(MAKE) -C tests

dummy:

$(LIBC): transfer .config.dir
	@for i in `cat .config.dir` ; do \
	   echo $(MAKE) -C $$i libc.a ; $(MAKE) -C $$i libc.a || exit 1 ; \
	done

transfer: .config.dir
	@echo Checking for transfers
	@for i in `cat .config.dir`; do \
	   grep -q '^transfer' $$i/Makefile && $(MAKE) -s -C $$i $@ ; \
	done ; echo -n
	@[ -f kinclude/Used ] || \
	{ rm -f include/linuxmt ; \
	  ln -s $(ELKSSRC)/include/linuxmt include ; }

realclean: noconfig clean dellib

clean:
	rm -f $(OBJ) $(LIBC)
	@for i in */Makefile ; do \
	   $(MAKE) -C `dirname $$i` $@ || exit 1 ; \
	done

dellib:
	rm -f libc*.a libdos.a

##############################################################################

.config.lst: Makefile Make.defs Config_sh
	sh Config_sh
	
config:
	sh Config_sh

noconfig:
	rm -f .config.dir .config.lst .config.otype

.config.dir: .config.lst
	@grep '^[^:]*:+:' < .config.lst | sed 's/:.*//' > .config.dir

.config.otype: dummy
	@[ -f .config.otype ] || echo $(PLATFORM) > .config.otype
	@[ "$(PLATFORM)" = "`cat .config.otype `" ] || $(MAKE) -$(MAKEFLAGS) clean
	@rm -f .config.otype
	@echo $(PLATFORM) > .config.otype

dist: clean
	-rm -f include/linuxmt
	tar cf temp.tar \
		$(TXT) $(SRC) include \
		`for i in */Makefile */Config; do dirname $$i; done | sort -u`
	rm -rf libc-$(VER)
	mkdir libc-$(VER) ; cd libc-$(VER) ; tar xf ../temp.tar
	tar czf libc-8086-$(VER).tar.gz libc-$(VER)
	rm -rf libc-$(VER) temp.tar

dist_ver: dist
	mv libc-8086-$(VER).tar.gz ..
	echo $(VER) > ../Libc_version
