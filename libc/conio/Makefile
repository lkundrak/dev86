# Copyright (C) 1996 Robert de Bath <robert@mayday.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

ASRC=conio.c
AOBJ=getch.o getche.o kbhit.o putch.o cputs.o gotoxy.o

BIOSOBJ=cprintf.o
DOSOBJ=$(AOBJ) cprintf.o

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

ifeq ($(LIB_CPU)-$(LIB_OS),i86-BIOS)
all: $(LIBC)($(BIOSOBJ))
	@$(RM) $(BIOSOBJ)
else
ifeq ($(LIB_CPU)-$(LIB_OS),i86-DOS)
all: $(LIBC)($(DOSOBJ))
	@$(RM) $(DOSOBJ)
else
all:
	@:
endif
endif

$(LIBC)($(AOBJ)): $(ASRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(BOBJ)): $(BSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)(cprintf.o): cprintf.c
	$(CC) -c -ansi $(ARCH) $(CCFLAGS) $(DEFS) $*.c
	$(AR) $(ARFLAGS) $@ $*.o

clean:
	rm -f *.o libc.a
