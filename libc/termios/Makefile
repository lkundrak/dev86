# Copyright (C) 1996 Robert de Bath <robert@mayday.compulink.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

TSRC=termios.c
TOBJ=tcsetattr.o tcgetattr.o tcdrain.o tcflow.o tcflush.o tcsendbreak.o	\
     tcsetpgrp.o tcgetpgrp.o isatty.o	\
     cfgetospeed.o cfgetispeed.o cfsetospeed.o cfsetispeed.o cfmakeraw.o

ifeq ($(LIB_OS),ELKS)
OBJ=$(TOBJ) ttyname.o

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

all: $(LIBC)
	@$(RM) $(OBJ)

$(LIBC): $(LIBC)($(OBJ))

$(LIBC)($(TOBJ)): $(TSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o
else
all:
	@:
endif

clean:
	rm -f *.o libc.a
