# Copyright (C) 1996 Robert de Bath <robert@mayday.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

TSRC=termios.c
TOBJ=tcsetattr.o tcgetattr.o tcdrain.o tcflow.o tcflush.o tcsendbreak.o	\
     tcsetpgrp.o tcgetpgrp.o isatty.o	\
     cfgetospeed.o cfgetispeed.o cfsetospeed.o cfsetispeed.o cfmakeraw.o

ifeq ($(LIB_OS),ELKS)
OBJ=$(TOBJ) ttyname.o
else
OBJ=
endif

all: $(OBJ)

libc.a: $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

clean:
	rm -f *.o libc.a

$(TOBJ): $(TSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(TSRC)

