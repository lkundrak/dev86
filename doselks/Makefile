
CC=$(BCC)
BCC=bcc
CFLAGS=-Md
LDFLAGS=-Md -s

DISTFILES=Makefile syscalls.h doselks.h coroutine.c doselks.c syscalls.c

# NB coroutine.o first then we can find it easily for breakpoints in CV.EXE
OBJ=coroutine.o doselks.o syscalls.o

all: doselks.com

install:
	# Err, Ok, but where do I install a dos program ?

doselks.com: $(OBJ)
	$(CC) $(LDFLAGS) -o doselks.com $(OBJ) 

syscalls.o: call_tab.v
$(OBJ): doselks.h

call_tab.v:	dummy
	-cp -p ../libc/syscall/call_tab.v . 2>/dev/null
	-cp -p ../libc/syscall/defn_tab.v . 2>/dev/null

distribution:
	tar czf /tmp/doselks.tar.gz $(DISTFILES)

dummy:

clean:
	rm -f $(OBJ) doselks.com

realclean:
	rm -f call_tab.v defn_tab.v
	rm -f $(OBJ) doselks.com

VVV=xx
$(VVV).o: $(VVV).c
	$(CC) $(CFLAGS) -A-l -A$(VVV).lst -c $(VVV).c

