# Copyright (C) 1996,1997 Robert de Bath <robert@mayday.cix.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

ifeq ($(TOPDIR),)
# This should work, but ..
TOP=..
else
TOP=$(TOPDIR)/libc
endif

VERMAJOR=0
VERMINOR=12
VERPATCH=4
VER=$(VERMAJOR).$(VERMINOR).$(VERPATCH)

CC=bcc
CCFLAGS=-I -I$(TOP)/include
DEFS=-D__LIBC__

include Make.defs

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

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
	$(CC) -c $(CFLAGS) -D__LIBC_VER__='"$(VER)"' -o $@ crt0.c

crt0.o: crt0.c Makefile
	$(CC) -c $(CFLAGS) -D__LIBC_VER__='"$(VER)"' -o $@ crt0.c

crtX.o:
	@echo "You need to define the 'PLATFORM=...' variable,"
	@echo "Preferably by doing make from `dirname \`pwd\``"
	@exit 1

############################################################################

transfer: .config.dir
	@for i in `cat .config.dir`; do \
	   grep -s '^transfer' $$i/Makefile && $(MAKE) -s -C $$i $@ ; \
	done ; echo -n
	@[ -f kinclude/Used ] || \
	{ rm -f include/linuxmt ; \
	  ln -s $(ELKSSRC)/include/linuxmt include ; }

############################################################################

realclean: clean noconfig

clean:
	rm -f *.o *.a
	@for i in */Makefile ; do \
	   $(MAKE) -C `dirname $$i` $@ || exit 1 ; \
	done

############################################################################

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

Libc_version:
	echo $(VER) > ../Libc_version
