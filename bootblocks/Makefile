
HOSTCC=cc
HOSTCCFLAGS=-O

CC=bcc
CFLAGS=-ansi -O -Ms
AS=as86
ASFLAGS=-0
# LST=-l $*.lst

nothing:
	@more README

all: makeboot monitor.out bin

CSRC=minix.c
SSRC=tarboot.s tiny.s com_bcc.s tich.s sysboot.s bootlist.s

encap: $(SSRC:s=v) $(CSRC:c=v)
bin: $(SSRC:s=bin) $(CSRC:c=bin)

MOBJ=monitor.o i86_funcs.o relocate.o
MSRC=monitor.c i86_funcs.c relocate.c

install:

monitor.out: $(MOBJ)
	$(CC) $(CFLAGS) $(MOBJ) -o monitor.out

monitor: $(MSRC)
	@rm -f $(MOBJ)
	make 'CFLAGS=-ansi -O' monitor.out
	mv monitor.out monitor
	@rm -f $(MOBJ)

minix.s: minix.c
	bcc -Mf -O -S minix.c

makeboot:	tarboot.v sysboot.v makeboot.c
	$(HOSTCC) $(HOSTCCFLAGS) -o makeboot makeboot.c

clean:
	rm -f monitor makeboot minix.s *.o *.bin *.out *.lst *.sym *.v

tgz: minix.bin monitor.out makeboot
	tar cfV bootblocks.tar ENIAC monitor.out \
	        README Makefile \
		$(MSRC) standalone.c makeboot.c \
		$(CSRC) $(SSRC) \
		makeboot minix.bin
	makeboot bootblocks.tar
	gzip -f9 bootblocks.tar

distribution:
	tar czf /tmp/bootblocks.tar.gz README Makefile \
		$(MSRC) standalone.c makeboot.c \
		$(CSRC) $(SSRC)

.SUFFIXES: .bin .v

.s.bin:
	$(AS) $(ASFLAGS) -b $*.bin -s $*.sym -l $*.lst $*.s

.s.v:
	as86_encap $*.s $*.v $*_ $(ASFLAGS) $(LST)
