# Copyright (C) 1996 Robert de Bath <robert@mayday.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

ifeq ($(LIB_OS),BIOS)
ASRC=bios.c
AOBJ=bios_start.o bios_isatty.o \
     bios_open.o bios_read.o bios_write.o bios_lseek.o bios_close.o

BSRC=bios_vid.c
BOBJ=bios_putc.o bios_getc.o bios_rdline.o
endif

all: $(AOBJ) $(BOBJ)

libc.a: $(AOBJ) $(BOBJ)
	ar r ../$(LIBC) $(AOBJ) $(BOBJ)
	@touch libc.a

clean:
	rm -f *.o libc.a

$(AOBJ): $(ASRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(ASRC)

$(BOBJ): $(BSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(BSRC)
