
HOSTCC=cc
HOSTCCFLAGS=-O
BCC=bcc

CC=$(BCC)
CFLAGS=-ansi -Ms -Oi -O -s $(MONDEFS)
# CFLAGS=-ansi -Ms
ASFLAGS=-0 -w
MINIXDEFS=-DDOTS
MONDEFS=
# LST=-l $*.lst
# CLST=-A-l -A$*.lst

default: makeboot makeboot.com monitor.out minix_elks.bin

all: default tgz bin  minix_elks.bin

CSRC=minix.c
SSRC=sysboot.s \
     tarboot.s skip.s com_bcc.s tich.s \
     bootlist.s mbr.s msdos.s noboot.s

encap: $(SSRC:s=v) $(CSRC:c=v) minixhd.v
bin: $(SSRC:s=bin) $(CSRC:c=bin) minixhd.bin

MOBJ=monitor.o commands.o i86_funcs.o relocate.o help.o bzimage.o \
     trk_buf.o min_buf.o unix.o fs.o fs_tar.o fs_min.o fs_dos.o
MSRC=monitor.c commands.c i86_funcs.c relocate.c help.c bzimage.c \
     trk_buf.c min_buf.c unix.c fs.c fs_tar.c fs_min.c fs_dos.c
MINC=i86_funcs.h readfs.h monitor.h

BOOTBLOCKS=sysboot.v noboot.v skip.v msdos.v tarboot.v minix.v minixhd.v mbr.v

EXTRAS=minix.h elf_info.c elf_info.h standalone.c li86.s \
       zimage.s minix_elks.c crc.c lsys.c

install:

monitor.out: $(MOBJ)
	$(CC) $(CFLAGS) -H0x10000 $(MOBJ) -o monitor.out -M > monitor.sym

$(MOBJ): $(MINC) version.h

monitor: $(MSRC) $(MINC)
	@rm -f $(MOBJ)
	make 'CFLAGS=-ansi -H0x8000 $(MONDEFS)' monitor.out
	mv monitor.out monitor
	@rm -f $(MOBJ)

bzimage.o: bzimage.c zimage.v

minix.s: minix.c Makefile
	$(BCC) -Mf -O -DTRY_FLOPPY $(MINIXDEFS) -S minix.c

minix_elks.s: minix_elks.c Makefile minix.v
	$(BCC) -Mf -O $(MINIXDEFS) -S minix_elks.c

minixhd.s: minix.c Makefile
	$(BCC) -Mf -O -DHARDDISK $(MINIXDEFS) -S minix.c -o minixhd.s

makeboot:	makeboot.c $(BOOTBLOCKS)
	$(HOSTCC) $(HOSTCCFLAGS) -o makeboot makeboot.c

makeboot.com:	makeboot.c $(BOOTBLOCKS)
	$(BCC) -Md -O -o makeboot.com makeboot.c

lsys.com:	lsys.c msdos.v
	$(BCC) -Md -O -o lsys.com lsys.c

version.h:
	head -1 ../Libc_version | \
	sed 's/\(.*\)/#define VERSION "\1"/' > version.h

clean realclean:
	rm -f monitor makeboot bootblocks.tar.gz
	rm -f minix.s minixhd.s minix_elks.s version.h
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
