# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

ASRC=msdos.c
AOBJ= dos_start.o __mkargv.o dos__fconv.o dos_read.o dos_write.o	\
      dos_open.o dos_close.o dos_unlink.o dos_lseek.o			\
      dos_segalloc.o dos_setvect.o dos_getvect.o dos_isatty.o

BSRC=i86.c
BOBJ= __set_es.o __peek_es.o __poke_es.o __deek_es.o __strchr_es.o

OBJ=$(AOBJ) $(BOBJ)

all: $(OBJ)

libc.a: $(BOBJ)
	ar r ../$(LIBC) $(BOBJ)
	@touch libc.a

libdos.a: $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

clean:
	rm -f $(OBJ) libc.a

$(AOBJ): $(ASRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(ASRC)

$(BOBJ): $(BSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(BSRC)

