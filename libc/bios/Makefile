# Copyright (C) 1996 Robert de Bath <robert@mayday.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

ifeq ($(LIB_OS),BIOS)
ASRC=bios.c
AOBJ=bios_start.o bios_isatty.o bios_nofiles.o \
     bios_read.o bios_write.o bios_lseek.o bios_close.o

BSRC=bios_vid.c
BOBJ=bios_putc.o bios_getc.o bios_getce.o bios_khit.o bios_rdline.o

OBJ=$(AOBJ) $(BOBJ) time.o fileops.o fs_dos.o rawio.o

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

all: $(LIBC)($(OBJ))
	@$(RM) $(OBJ)

$(LIBC)($(AOBJ)): $(ASRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(BOBJ)): $(BSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o
else
all:
	@:
endif

clean:
	rm -f *.o libc.a
