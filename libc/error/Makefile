# Copyright (C) 1996 Robert de Bath <robert@debath.thenet.co.uk>
# This file is part of the Linux-8086 C library and is distributed
# under the GNU Library General Public License.

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

ifeq ($(LIB_OS),ELKS)

ifneq ($(LIB_CPU),i86)
OBJ=error2.o perror.o sys_siglist.o __assert.o
else
OBJ=error.o sys_errlist.o perror.o sys_siglist.o __assert.o
endif

else
OBJ=__assert.o
endif

all: $(LIBC)($(OBJ))
	@$(RM) $(OBJ)

clean:
	rm -f *.o libc.a error_list.h

$(LIBC)(error2.o): error_list.h

error_list.h: liberror.txt
	sh mktab.sh
