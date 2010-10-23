
HOSTCC=cc
HOSTCCFLAGS=-O
BCC=bcc
AS86=as86

DEFS=
CC=$(BCC)
CFLAGS=-ansi -Ms -Oi -O $(DEFS)
LDFLAGS=-s -i -H0x10000 
ASFLAGS=-0 -w
MINIXDEFS=-DDOTS
# CFLAGS=-ansi -Ms $(DEFS)
LST=-l $*.lst

default: makeboot makeboot.com monitor.sys minix_elks.bin lsys.com

all: bootbin bootsys default tgz

bootsys: bootfile.sys boottar.sys bootminix.sys monitor.sys boot_win.sys

CSRC=minix.c
SSRC=tarboot.s skip.s mbr.s mbr_dm.s msdos.s noboot.s nombr.s \
     boot_fpy.s killhd.s bb_linux.s bb_init1.s bb_init2.s
SINC=sysboot16.s sysboot.s sysmbr.s sysmbrtail.s

encap: $(SSRC:s=v) $(CSRC:c=v) minixhd.v mbr_chs.v mbr_lin.v mbr_lba.v pbr.v msdos16.v
bootbin: $(SSRC:s=bin) $(CSRC:c=bin) minixhd.bin mbr_chs.bin mbr_lin.bin mbr_lba.bin pbr.bin msdos16.bin minix_elks.bin

MOBJ=monitor.o commands.o i86_funcs.o relocate.o help.o bzimage.o \
     buffer.o unix.o fs.o fs_tar.o fs_min.o fs_dos.o
MSRC=monitor.c commands.c i86_funcs.c relocate.c help.c bzimage.c \
     buffer.c unix.c fs.c fs_tar.c fs_min.c fs_dos.c
MINC=i86_funcs.h readfs.h monitor.h

BOOTBLOCKS=sysboot.v noboot.v skip.v msdos.v msdos16.v \
           tarboot.v minix.v minixhd.v mbr.v killhd.v \
	   sysboot16.v nombr.v mbr_dm.v mbr_chs.v mbr_lin.v mbr_lba.v \
	   pbr.v

EXTRAS=minix.h zimage.s minix_elks.c lsys.c boot_win.c \
       freedosboot.zip freedos.h

install: makeboot
	install -m 755 -s makeboot $(DIST)$(PREFIX)/bin/makeboot

monitor.com: $(MOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(MONDEFS) -d $(MOBJ) -o monitor.com -M > monitor.sym

monitor.out: $(MOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(MONDEFS) $(MOBJ) -o monitor.out -M > monitor.sym

$(MOBJ): $(MINC)
fs_min.o: minix.h

bootfile.sys: $(MSRC) $(MINC)
	@rm -f $(MOBJ)
	make 'CFLAGS=$(CFLAGS) -DDOSFLOPPY' monitor.out
	mv monitor.out bootfile.sys
	@rm -f $(MOBJ)

boottar.sys: $(MSRC) $(MINC) tarboot.bin
	@rm -f $(MOBJ)
	make 'CFLAGS=$(CFLAGS) -DTARFLOPPY' monitor.out
	mv monitor.out boottar.sys
	@rm -f $(MOBJ)

bootminix.sys: $(MSRC) $(MINC) minix.bin
	@rm -f $(MOBJ)
	make 'CFLAGS=$(CFLAGS) -DMINFLOPPY' monitor.out
	mv monitor.out bootminix.sys
	@rm -f $(MOBJ)

monitor.sys: $(MSRC) $(MINC)
	@rm -f $(MOBJ)
	make monitor.out
	mv monitor.out monitor.sys
	@rm -f $(MOBJ)

monitor: $(MSRC) $(MINC)
	@rm -f $(MOBJ)
	make 'CFLAGS=-ansi $(DEFS)' monitor.out
	mv monitor.out monitor
	@rm -f $(MOBJ)

bzimage.o: bzimage.c zimage.v

minix.s: minix.c Makefile
	$(CC) -Mf -O -DTRY_FLOPPY $(MINIXDEFS) -S minix.c

minix_elks.s: minix_elks.c Makefile minix.v
	$(CC) -Mf -O $(MINIXDEFS) -S minix_elks.c

minixhd.s: minix.c Makefile
	$(CC) -Mf -O -DHARDDISK $(MINIXDEFS) -S minix.c -o minixhd.s

msdos16.s: msdos.s Makefile
	sed 's/^fatbits=12/fatbits=16/' < msdos.s > msdos16.s

mbr_chs.s: mbr.s Makefile
	sed -e 's/^linear=1/linear=0/' \
	    -e 's/^useCHS=0/useCHS=1/' \
	    -e 's/^linCHS=1/linCHS=0/' \
	    -e 's/^mbrkey=1/mbrkey=0/' \
	    -e 's/^preboot=1/preboot=0/' \
	    -e 's/^pbr=1/pbr=0/' \
	    -e 's/^direct=1/direct=0/' \
	    -e 's/^message=1/message=0/' \
	     < mbr.s > mbr_chs.s

mbr_lin.s: mbr.s Makefile
	sed -e 's/^linear=1/linear=0/' \
	    -e 's/^useCHS=0/useCHS=1/' \
	    -e 's/^linCHS=0/linCHS=1/' \
	    -e 's/^mbrkey=1/mbrkey=0/' \
	    -e 's/^preboot=1/preboot=0/' \
	    -e 's/^pbr=1/pbr=0/' \
	    -e 's/^direct=1/direct=0/' \
	    -e 's/^message=1/message=0/' \
	     < mbr.s > mbr_lin.s

pbr.s: mbr.s Makefile
	sed -e 's/^linear=0/linear=1/' \
	    -e 's/^useCHS=1/useCHS=0/' \
	    -e 's/^linCHS=1/linCHS=0/' \
	    -e 's/^mbrkey=1/mbrkey=0/' \
	    -e 's/^preboot=1/preboot=0/' \
	    -e 's/^pbr=0/pbr=1/' \
	    -e 's/^direct=1/direct=0/' \
	    -e 's/^message=1/message=0/' \
	     < mbr.s > pbr.s

mbr_lba.s: mbr.s Makefile
	sed -e 's/^linear=0/linear=1/' \
	    -e 's/^useCHS=1/useCHS=0/' \
	    -e 's/^linCHS=1/linCHS=0/' \
	    -e 's/^mbrkey=1/mbrkey=0/' \
	    -e 's/^preboot=1/preboot=0/' \
	    -e 's/^pbr=1/pbr=0/' \
	    -e 's/^direct=1/direct=0/' \
	    -e 's/^message=1/message=0/' \
	     < mbr.s > mbr_lba.s

boot_win.sys: boot_win.c
	$(CC) -Ms -H0x6000 -s boot_win.c -o boot_win.sys

makeboot:	makeboot.c $(BOOTBLOCKS) freedos.h
	$(HOSTCC) $(HOSTCCFLAGS) -o makeboot makeboot.c

ldboot:	ldboot.c
	$(HOSTCC) $(HOSTCCFLAGS) -o ldboot ldboot.c

makeboot.com:	makeboot.c $(BOOTBLOCKS) freedos.h
	$(CC) -Md -O -o makeboot.com makeboot.c

lsys.com:	lsys.c msdos.v msdos16.v
	$(CC) -Md -O -o lsys.com lsys.c

clean realclean:
	rm -f bootfile.sys boottar.sys bootminix.sys monitor.sys boot_win.sys
	rm -f monitor makeboot bootblocks.tar.gz
	rm -f minix.s minixhd.s minix_elks.s msdos16.s mbr_chs.s mbr_lin.s mbr_lba.s pbr.s
	rm -f *.com *.o *.bin *.out *.lst *.sym *.v *.tmp

tgz: minix.bin monitor.sys makeboot.com makeboot
	tar cfV bootblocks.tar ENIAC monitor.sys \
	        README Makefile \
		$(MSRC) \
		$(MINC) \
		makeboot.c $(CSRC) freedos.h \
		$(SSRC) $(SINC) \
		makeboot.com minix.bin \
		$(EXTRAS)
	makeboot tar bootblocks.tar
	gzip -f9 bootblocks.tar

distribution:
	tar czf /tmp/bootblocks.tar.gz \
	   README Makefile $(MSRC) $(MINC) makeboot.c $(CSRC) $(SSRC) $(SINC) $(EXTRAS)

.SUFFIXES: .bin .v

.s.bin:
	$(AS86) -w- -0 -b $*.bin -s $*.sym $*.s $(LST)

.s.v:
	as86_encap $*.s $*.v $*_ $(ASFLAGS) $(LST)
