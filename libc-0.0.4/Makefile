# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=.
include $(TOP)/Make.defs

SRC=crt0.c
OBJ=crt0.o

TARGETS=$(OBJ) $(LIBC)
TXT=COPYING Changes Contributors MAGIC \
    Make.defs Makefile New_subdir Pre_main README TODO KERNEL

all: $(TARGETS)

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

$(LIBC): transfer
	for i in */Makefile ; do \
	   make -C `dirname $$i` $@ || exit 1 ; \
	done

realclean: clean
	rm -f $(OBJ) $(LIBC) libc-$(VER)
	@for i in */makefile ; do \
	   grep -q '^clean:' $$i && make -C `dirname $$i` clean ; \
	done ; echo -n

clean: dummy
	for i in */Makefile ; do \
	   make -C `dirname $$i` $@ || exit 1 ; \
	done

config:
	sh Config.sh
	@for i in */Makefile ; do \
	   [ -f `dirname $$i`/Config.sh ] && \
	     sh `dirname $$i`/Config.sh ] ; \
	done ; echo -n

dist: clean
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
	@for i in */Makefile ; do \
	   grep -q '^transfer' $$i && make -C `dirname $$i` $@ ; \
	done ; echo -n
	
dummy:
