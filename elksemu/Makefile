#
#	Makefile for elksemu.
#

CFLAGS=-O2 -fno-strength-reduce -Wall
# If you need an a.out exe. NB The program _does_ now work as ELF
# CFLAGS=-O2 -fno-strength-reduce -b i486-linuxaout -N -s -static

OBJ=elks.o elks_sys.o elks_signal.o

elksemu:	$(OBJ)
		$(CC) $(CFLAGS) -o elksemu $(OBJ)

$(OBJ):		elks.h
elks_sys.o:	call_tab.v

call_tab.v:	dummy
	-cp -p ../libc/syscall/call_tab.v . 2>/dev/null
	-cp -p ../libc/syscall/defn_tab.v . 2>/dev/null

dummy:

# The kernel patch _requires_ this location.
install: elksemu
	install -s -m 755 elksemu /lib/elksemu
	tar cvf V-files.tar call_tab.v defn_tab.v

clean:
	rm -f $(OBJ) elksemu call_tab.v defn_tab.v
