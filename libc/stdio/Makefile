# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

ifneq ($(LIB_CPU),i86)
CFLAGS=$(CCFLAGS) $(LIBDEFS) -DFLOATS
endif

ASRC=stdio.c
AOBJ=_stdio_init.o fputc.o fgetc.o fflush.o fgets.o gets.o fputs.o 	\
     puts.o fread.o fwrite.o fopen.o fdopen.o freopen.o __fopen.o 	\
     fclose.o fseek.o rewind.o ftell.o setbuffer.o setvbuf.o ungetc.o

PSRC=printf.c
POBJ=printf.o sprintf.o fprintf.o vprintf.o vsprintf.o vfprintf.o fp_print.o

SSRC=scanf.c
SOBJ=scanf.o sscanf.o fscanf.o vscanf.o vsscanf.o vfscanf.o

OBJ= $(AOBJ) $(POBJ) $(SOBJ)

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

all: $(LIBC)
	@$(RM) $(OBJ)

$(LIBC): $(LIBC)($(OBJ))

$(LIBC)($(AOBJ)): $(ASRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(POBJ)): $(PSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)($(SOBJ)): $(SSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

transfer:
	-@rm -f ../include/stdio.h
	cp -p stdio.h ../include/.

clean:
	rm -f *.o libc.a ../include/stdio.h

$(LIBC)($(OBJ)): stdio.h

