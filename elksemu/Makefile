#
#	Makefile for elksemu.
#

CFLAGS=-O2 -fno-strength-reduce -Wall
# If you need an a.out exe. NB The program _does_ now work as ELF
# CFLAGS=-O2 -fno-strength-reduce -b i486-linuxaout -N -s -static

OBJ=elks.o elks_sys.o elks_signal.o

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
	install -s -o root -g root -m 4111 elksemu /lib/elksemu
	tar cvf V-files.tar call_tab.v defn_tab.v

clean:
	rm -f $(OBJ) elksemu call_tab.v defn_tab.v

module: binfmt_elks.o

# HOW to compile the module...

# This matches my compile; yours may be different.
binfmt_elks.o:	binfmt_elks.c
	gcc -O -c -fomit-frame-pointer -DCPU=386 -D__KERNEL__ -DMODULE -DMODVERSIONS -include /usr/include/linux/modversions.h binfmt_elks.c

# This is another option
#	gcc -O -c -fomit-frame-pointer -DCPU=386 -D__KERNEL__ -DMODULE binfmt_elks.c
# Or this ...
#	gcc -O -c -fomit-frame-pointer -D__KERNEL__ -DMODULE -DCONFIG_MODVERSIONS binfmt_elks.c

