# Copyright (C) 1996 Robert de Bath <robert@mayday.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TOP=..
include $(TOP)/Make.defs

TSRC=termios.c
TOBJ=tcsetattr.o tcgetattr.o tcdrain.o tcflow.o tcflush.o tcsendbreak.o	\
     tcsetpgrp.o tcgetpgrp.o isatty.o	\
     cfgetospeed.o cfgetispeed.o cfsetospeed.o cfsetispeed.o

OBJ=$(TOBJ) ttyname.o

all: $(OBJ)

ifeq ($(PLATFORM),i86-DOS)
libc.a:
else
libc.a: $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a
endif

clean:
	rm -f *.o libc.a

$(TOBJ): $(TSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(TSRC)

