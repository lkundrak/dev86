
HOSTCC=cc
HOSTCCFLAGS=-O
BCC=bcc

CC=$(BCC)
CFLAGS=-ansi -Ms -H0x10000 -s
ASFLAGS=-0
# LST=-l $*.lst

default: makeboot makeboot.com monitor.out

all: default tgz bin 

CSRC=minix.c
SSRC=tarboot.s skip.s com_bcc.s tich.s sysboot.s bootlist.s mbr.s msdos.s

encap: $(SSRC:s=v) $(CSRC:c=v)
bin: $(SSRC:s=bin) $(CSRC:c=bin)

MOBJ=monitor.o i86_funcs.o relocate.o help.o bzimage.o dosfs.o nofs.o
MSRC=monitor.c i86_funcs.c relocate.c help.c bzimage.c dosfs.c nofs.c

install:

monitor.out: $(MOBJ)
	$(CC) $(CFLAGS) $(MOBJ) -o monitor.out

monitor: $(MSRC)
	@rm -f $(MOBJ)
	make 'CFLAGS=-ansi' monitor.out
	mv monitor.out monitor
	@rm -f $(MOBJ)

minix.s: minix.c
	$(BCC) -Mc -S minix.c

makeboot:	makeboot.c sysboot.v skip.v msdos.v tarboot.v
	$(HOSTCC) $(HOSTCCFLAGS) -o makeboot makeboot.c

makeboot.com:	makeboot.c sysboot.v skip.v msdos.v tarboot.v
	$(BCC) -Md -o makeboot.com makeboot.c

clean realclean:
	rm -f monitor makeboot bootblocks.tar.gz
	rm -f minix.s *.com *.o *.bin *.out *.lst *.sym *.v

tgz: minix.bin monitor.out makeboot.com makeboot
	tar cfV bootblocks.tar ENIAC monitor.out \
	        README Makefile \
		$(MSRC) makeboot.c \
		$(CSRC) $(SSRC) \
		makeboot.com minix.bin
	makeboot tar bootblocks.tar
	gzip -f9 bootblocks.tar

distribution:
	tar czf /tmp/bootblocks.tar.gz README Makefile \
		$(MSRC) makeboot.c \
		$(CSRC) $(SSRC)

.SUFFIXES: .bin .v

.s.bin:
	$(BCC) -c $*.s -A-b -A$*.bin -A-s -A$*.sym -A-l -A$*.lst

.s.v:
	as86_encap $*.s $*.v $*_ $(ASFLAGS) $(LST)
