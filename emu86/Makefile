
CFLAGS = -g
PREFIX = /usr/local

# EMU86 program

EMU86_PROG = emu86

EMU86_HDRS = op-common.h op-id-name.h op-exec.h op-class.h
EMU86_SRCS = op-common.c op-id-name.c op-exec.c op-class.c emu-mem-io.c emu-proc.c emu-int.c op-main.c
EMU86_OBJS = op-common.o op-id-name.o op-exec.o op-class.o  emu-mem-io.o emu-proc.o emu-int.c op-main.o

$(EMU86_OBJS): $(EMU86_HDRS) 

$(EMU86_PROG): $(EMU86_OBJS)
	$(CC) -o $(EMU86_PROG) $(EMU86_OBJS)

all: $(EMU86_PROG)

install: $(EMU86_PROG)
	install -m 755 -s $(EMU86_PROG) $(PREFIX)/bin/$(EMU86_PROG)

clean:	
	rm -f $(EMU86_OBJS) $(EMU86_PROG)
	