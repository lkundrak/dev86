# Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

SSRC=string.c
SOBJ=strlen.o strcat.o strcpy.o strcmp.o strncat.o strncpy.o strncmp.o	\
    strchr.o strrchr.o strdup.o memcpy.o memccpy.o memchr.o memset.o	\
    memcmp.o memmove.o movedata.o

OBJ=$(SOBJ) strpbrk.o strsep.o strstr.o strtok.o strcspn.o	\
    strspn.o strcasecmp.o strncasecmp.o

all: $(OBJ)

libc.a: $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

transfer:
	-@rm -f ../include/string.h
	cp -p string.h ../include/.

clean:
	rm -f *.o libc.a

$(SOBJ): $(SSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(SSRC)

