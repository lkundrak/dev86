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

all: .config.lst $(TARGETS)

install: all
	install -d $(BCCHOME)
	rm -rf $(BCCHOME)/include
	ln -s $(TOPDIR)/libc/include $(BCCHOME)/include
	install -d $(LIBDIR)/i86
	install -m 644 crt0.o $(LIBDIR)/i86
	install -m 644 $(LIBC) $(LIBDIR)/i86
	-install -m 644 error/liberror.txt /usr/lib

tests: dummy
	make -C tests

dummy:

$(LIBC): .config.dir transfer
	@for i in `cat .config.dir` ; do \
	   echo make -C $$i libc.a ; make -C $$i libc.a || exit 1 ; \
	done

realclean: dummy
	rm -f $(OBJ) $(LIBC) .config.dir
	@for i in */Makefile ; do \
	   grep -q '^clean:' $$i && make -C `dirname $$i` clean ; \
	done ; echo -n

clean: .config.dir
	@for i in */Makefile ; do \
	   make -C `dirname $$i` $@ || exit 1 ; \
	done

.config.lst: Makefile Make.defs Config_sh
	sh Config_sh
	
config:
	sh Config_sh

.config.dir: .config.lst
	@grep '^[^:]*:+:' < .config.lst | sed 's/:.*//' > .config.dir

dist: clean
	-rm -f include/linuxmt
	tar cf temp.tar \
		$(TXT) $(TARGETS) $(SRC) include \
		`for i in */Makefile */Config; do dirname $$i; done | sort -u`
	rm -rf libc-$(VER)
	mkdir libc-$(VER) ; cd libc-$(VER) ; tar xf ../temp.tar
	tar czf libc-8086-$(VER).tar.gz libc-$(VER)
	rm -rf libc-$(VER) temp.tar

dist_ver: dist
	mv libc-8086-$(VER).tar.gz ..
	echo $(VER) > ../Libc_version

transfer: dummy
	@echo Checking for transfers
	@for i in `cat .config.dir`; do \
	   grep -q '^transfer' $$i/Makefile && make -C $$i $@ ; \
	done ; echo -n
