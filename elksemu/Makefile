#
#	Makefile for elksemu.
#

ifeq ($(CC),bcc)
# Use BCC to make a tiny static a.out version.
CFLAGS=-O -3 -N -ansi -s
else
# For gcc with the default compiler
CFLAGS=-O2 -fno-strength-reduce -Wall -idirafter . $(DEFS)
endif

# For gcc making a.out with a basically ELF compiler
# CFLAGS=-O2 -fno-strength-reduce -b i486-linuxaout -N -s -static

OBJ=elks.o elks_sys.o elks_signal.o minix.o

elksemu:	$(OBJ)
		$(CC) $(CFLAGS) -o elksemu $(OBJ)

elks_sys.o:	call_tab.v
$(OBJ):		elks.h

call_tab.v:	dummy
	-cp -p ../libc/syscall/call_tab.v . 2>/dev/null
	-cp -p ../libc/syscall/defn_tab.v . 2>/dev/null

dummy:

# The kernel patch or module _requires_ this location.
install: elksemu
	install -d $(DIST)/lib
	install -s -o root -g root -m 4555 elksemu $(DIST)/lib/elksemu

clean realclean:
	rm -f $(OBJ) elksemu call_tab.v defn_tab.v

module: binfmt_elks.o

# HOW to compile the module...

# This matches my compile (2.0.0); yours may be different.
MODCFLAGS=-D__KERNEL__ -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer \
          -fno-strength-reduce -pipe -m486 -DCPU=486 -DMODULE -DMODVERSIONS \
	  -include /usr/include/linux/modversions.h

binfmt_elks.o:	binfmt_elks.c
	gcc -c $(MODCFLAGS) binfmt_elks.c

# This is another option
#MODCFLAGS=-O -fomit-frame-pointer -DCPU=386 -D__KERNEL__ -DMODULE
# Or this ...
#MODCFLAGS=-O -fomit-frame-pointer -D__KERNEL__ -DMODULE -DCONFIG_MODVERSIONS

