
HOSTCC=cc
HOSTCCFLAGS=-O
BCC=bcc
AS86=as86

DEFS=
CC=$(BCC)
CFLAGS=-ansi -Ms -Oi -O -s $(DEFS)
# CFLAGS=-ansi -Ms
ASFLAGS=-0 -w
MINIXDEFS=-DDOTS
# LST=-l $*.lst

default: makeboot makeboot.com monitor.out minix_elks.bin lsys.com

all: bootbin bootsys default tgz

bootsys: bootfile.sys boottar.sys bootminix.sys

CSRC=minix.c
SSRC=sysboot.s tarboot.s skip.s com_bcc.s tich.s mbr.s msdos.s noboot.s \
     boot_fpy.s killhd.s

encap: $(SSRC:s=v) $(CSRC:c=v) minixhd.v msdos16.v
bootbin: $(SSRC:s=bin) $(CSRC:c=bin) minixhd.bin msdos16.bin minix_elks.bin

MOBJ=monitor.o commands.o i86_funcs.o relocate.o help.o bzimage.o \
     trk_buf.o min_buf.o unix.o fs.o fs_tar.o fs_min.o fs_dos.o
MSRC=monitor.c commands.c i86_funcs.c relocate.c help.c bzimage.c \
     trk_buf.c min_buf.c unix.c fs.c fs_tar.c fs_min.c fs_dos.c
MINC=i86_funcs.h readfs.h monitor.h

BOOTBLOCKS=sysboot.v noboot.v skip.v msdos.v msdos16.v \
           tarboot.v minix.v minixhd.v mbr.v

EXTRAS=minix.h elf_info.c elf_info.h standalone.c li86.s \
       zimage.s minix_elks.c crc.c lsys.c

install:

monitor.com: $(MOBJ)
	$(CC) $(CFLAGS) $(MONDEFS) -H0x10000 -d $(MOBJ) -o monitor.com -M > monitor.sym

monitor.out: $(MOBJ)
	$(CC) $(CFLAGS) $(MONDEFS) -H0x10000 $(MOBJ) -o monitor.out -M > monitor.sym

$(MOBJ): $(MINC)
fs_min.o: minix.h

bootfile.sys: $(MSRC) $(MINC)
	@rm -f $(MOBJ)
	make 'CFLAGS=$(CFLAGS) -DDOSFLOPPY -i -d' monitor.out
	mv monitor.out bootfile.sys
	@rm -f $(MOBJ)

boottar.sys: $(MSRC) $(MINC) tarboot.bin
	@rm -f $(MOBJ)
	make 'CFLAGS=$(CFLAGS) -DTARFLOPPY -i' monitor.out
	mv monitor.out boottar.sys
	@rm -f $(MOBJ)

bootminix.sys: $(MSRC) $(MINC) tarboot.bin
	@rm -f $(MOBJ)
	make 'CFLAGS=$(CFLAGS) -DMINFLOPPY -i' monitor.out
	mv monitor.out bootminix.sys
	@rm -f $(MOBJ)

monitor: $(MSRC) $(MINC)
	@rm -f $(MOBJ)
	make 'CFLAGS=-ansi -H0x8000' monitor.out
	mv monitor.out monitor
	@rm -f $(MOBJ)

bzimage.o: bzimage.c zimage.v

minix.s: minix.c Makefile
	$(CC) -Mf -O -DTRY_FLOPPY $(MINIXDEFS) -S minix.c

minix_elks.s: minix_elks.c Makefile minix.v
	$(CC) -Mf -O $(MINIXDEFS) -S minix_elks.c

minixhd.s: minix.c Makefile
	$(CC) -Mf -O -DHARDDISK $(MINIXDEFS) -S minix.c -o minixhd.s

msdos16.s: msdos.s
	sed 's/^fatbits=12/fatbits=16/' < msdos.s > msdos16.s

makeboot:	makeboot.c $(BOOTBLOCKS)
	$(HOSTCC) $(HOSTCCFLAGS) -o makeboot makeboot.c

makeboot.com:	makeboot.c $(BOOTBLOCKS)
	$(CC) -Md -O -o makeboot.com makeboot.c

lsys.com:	lsys.c msdos.v msdos16.v
	$(CC) -Md -O -o lsys.com lsys.c

clean realclean:
	rm -f bootfile.sys boottar.sys bootminix.sys
	rm -f monitor makeboot bootblocks.tar.gz
	rm -f minix.s minixhd.s minix_elks.s msdos16.s
	rm -f *.com *.o *.bin *.out *.lst *.sym *.v *.tmp

tgz: minix.bin monitor.out makeboot.com makeboot
	tar cfV bootblocks.tar ENIAC monitor.out \
	        README Makefile \
		$(MSRC) \
		$(MINC) \
		makeboot.c $(CSRC) \
		$(SSRC) \
		makeboot.com minix.bin \
		$(EXTRAS)
	makeboot tar bootblocks.tar
	gzip -f9 bootblocks.tar

distribution:
	tar czf /tmp/bootblocks.tar.gz \
	   README Makefile $(MSRC) $(MINC) makeboot.c $(CSRC) $(SSRC) $(EXTRAS)

.SUFFIXES: .bin .v

.s.bin:
	$(AS86) -w- -0 -b $*.bin -s $*.sym $*.s $(LST)

.s.v:
	as86_encap $*.s $*.v $*_ $(ASFLAGS) $(LST)
