# Copyright (C) 1996,1997 Robert de Bath <robert@mayday.cix.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

ifeq ($(TOPDIR),)
# This should work, but ..
TOP=..
else
TOP=$(TOPDIR)/libc
endif

CC=bcc
CCFLAGS=-I -I$(TOP)/include
DEFS=-D__LIBC__

include Make.defs

ifeq ($(VERSION),)
include VERSION
endif

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)
USERID=$(shell id -gn)
GROUPID=$(shell id -un)

############################################################################

MAKEPASS=	\
         LIBC='$(LIBC)' CC='$(CC)' ARCH='$(ARCH)' CCFLAGS='$(CCFLAGS)'	\
         DEFS='$(DEFS)' LIB_CPU='$(LIB_CPU)' LIB_OS='$(LIB_OS)' \


all: $(OBJ) $(LIBC)
	@:

$(LIBC): transfer .config.dir
	@for i in `cat .config.dir` ; do \
	   $(MAKE) $(MAKEPASS) -C $$i all || exit 1 ; \
	done

crt3.o: crt0.c Makefile
	$(CC) -c $(CFLAGS) -D__LIBC_VER__='"$(VERSION)"' -o $@ crt0.c

crt0.o: crt0.c Makefile
	$(CC) -c $(CFLAGS) -D__LIBC_VER__='"$(VERSION)"' -o $@ crt0.c

crtg.o: crt0.c Makefile
	$(CC) -c $(CFLAGS) -D__LIBC_VER__='"$(VERSION)"' -o $@ crt0.c

crtX.o:
	@echo "You need to define the 'PLATFORM=...' variable,"
	@echo "Preferably by doing make from `dirname \`pwd\``"
	@exit 1

############################################################################

transfer: .config.dir
	@for i in `cat .config.dir`; do \
	   grep -s '^transfer' $$i/Makefile && $(MAKE) -s -C $$i $@ ; \
	done ; echo -n
	@[ -d include/linuxmt/. ] || \
	  ln -s $(ELKSSRC)/include/linuxmt include
	@[ -d include/arch/. ] || \
	  ln -s $(ELKSSRC)/include/arch include

############################################################################

realclean: clean noconfig

clean:
	rm -f *.o *.a
	@for i in */Makefile ; do \
	   $(MAKE) -C `dirname $$i` $@ || exit 1 ; \
	done

############################################################################

install_incl: transfer
	install -d $(DISTINCL)/include
	rm -f $(DISTINCL)/include/linuxmt $(DISTINCL)/include/arch ||:
	cp -LpR include/* $(DISTINCL)/include
	-chown -R $(USERID):$(GROUPID) $(DISTINCL)/include
	-chmod -R ugo-x,u=rwX,og=rX $(DISTINCL)/include

############################################################################

config: 
	sh Config_sh

.config.dir: .config.lst
	@grep '^[^:]*:+:' < .config.lst | sed 's/:.*//' > .config.tmp
	@mv -f .config.tmp .config.dir

.config.lst: Config_sh
	sh Config_sh
	
noconfig:
	rm -f .config.dir .config.lst .config.tmp

############################################################################
