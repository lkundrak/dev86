# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

ASRC=msdos.c
AOBJ= dos_start.o __mkargv.o __mkenvp.o dos__fconv.o dos_read.o		\
      dos_write.o dos_open.o dos_close.o dos_unlink.o dos_lseek.o	\
      dos_segalloc.o dos_segfree.o dos_setvect.o dos_getvect.o		\
      dos_isatty.o dos_getmod.o dos_stat.o

BSRC=i86.c
BOBJ= __seg_regs.o __peek_es.o __poke_es.o __deek_es.o __doke_es.o	\
      __strnget_es.o __strchr_es.o __strlen_es.o

ifeq ($(LIB_CPU),i86)
ifeq ($(LIB_OS),DOS)
OBJ=$(AOBJ) $(BOBJ) time.o
else
OBJ=$(BOBJ)
endif

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

all: $(LIBC)
	@$(RM) $(OBJ)

$(LIBC): $(LIBC)($(OBJ))

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

