# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

SSRC=string.c
SOBJ=strlen.o strcat.o strcpy.o strcmp.o strncat.o strncpy.o strncmp.o	\
    strchr.o strrchr.o strdup.o memcpy.o memccpy.o memchr.o memset.o	\
    memcmp.o memmove.o movedata.o

OBJ=$(SOBJ) strpbrk.o strsep.o strstr.o strtok.o strcspn.o	\
    strspn.o strcasecmp.o strncasecmp.o

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

all: $(LIBC)
	@$(RM) $(OBJ)

$(LIBC): $(LIBC)($(OBJ))

$(LIBC)($(SOBJ)): $(SSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

transfer:
	-@rm -f ../include/string.h
	cp -p string.h ../include/.

clean:
	rm -f *.o ../include/string.h
