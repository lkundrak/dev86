
HOSTCC=cc
HOSTCCFLAGS=-O
BCC=bcc

CC=$(BCC)
CFLAGS=-ansi -Ms -Oi -O -s
# CFLAGS=-ansi -Ms
ASFLAGS=-0 -w
MINIXDEFS=-DDOTS
MONDEFS=
# LST=-l $*.lst
# CLST=-A-l -A$*.lst

default: makeboot makeboot.com monitor.out minix_elks.bin lsys.com

all: bootbin bootsys default tgz

bootsys: bootfile.sys boottar.sys bootminix.sys

CSRC=minix.c
SSRC=sysboot.s tarboot.s skip.s com_bcc.s tich.s mbr.s msdos.s noboot.s \
     boot_fpy.s

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

monitor.out: $(MOBJ)
	$(CC) $(CFLAGS) $(MONDEFS) -H0x10000 $(MOBJ) -o monitor.out -M > monitor.sym

$(MOBJ): $(MINC) version.h
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
	$(BCC) -Mf -O -DTRY_FLOPPY $(MINIXDEFS) -S minix.c

minix_elks.s: minix_elks.c Makefile minix.v
	$(BCC) -Mf -O $(MINIXDEFS) -S minix_elks.c

minixhd.s: minix.c Makefile
	$(BCC) -Mf -O -DHARDDISK $(MINIXDEFS) -S minix.c -o minixhd.s

msdos16.s: msdos.s
	sed 's/^fatbits=12/fatbits=16/' < msdos.s > msdos16.s

makeboot:	makeboot.c $(BOOTBLOCKS)
	$(HOSTCC) $(HOSTCCFLAGS) -o makeboot makeboot.c

makeboot.com:	makeboot.c $(BOOTBLOCKS)
	$(BCC) -Md -O -o makeboot.com makeboot.c

lsys.com:	lsys.c msdos.v msdos16.v
	$(BCC) -Md -O -o lsys.com lsys.c

version.h:
	head -1 ../Libc_version | \
	sed 's/\(.*\)/#define VERSION "\1"/' > version.h

clean realclean:
	rm -f bootfile.sys boottar.sys bootminix.sys
	rm -f monitor makeboot bootblocks.tar.gz
	rm -f minix.s minixhd.s minix_elks.s version.h msdos16.s
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
	$(BCC) -W -c $*.s -A-u- -A-b -A$*.tmp -A-s -A$*.sym $(CLST)
	mv $*.tmp $*.bin
	-@rm $*.o

.s.v:
	as86_encap $*.s $*.v $*_ $(ASFLAGS) $(LST)
