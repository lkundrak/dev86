# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

ASRC=malloc.c
AOBJ=malloc.o alloca.o free.o calloc.o realloc.o

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

all: $(LIBC)($(AOBJ))
	@$(RM) $(OBJ)

$(LIBC)($(AOBJ)): $(ASRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

clean:
	rm -f *.o libc.a ../include/malloc.h

transfer:
	-@rm ../include/malloc.h
	cp -p malloc.h ../include/.
