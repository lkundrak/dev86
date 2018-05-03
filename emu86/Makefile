# EMU86 makefile

CFLAGS = -g
PREFIX = /usr/local

# EMU86 main program

EMU86_PROG = emu86

EMU86_HDRS = op-common.h op-id-name.h op-class.h emu-mem-io.h emu-proc.h emu-serial.h emu-int.h op-exec.h
EMU86_SRCS = op-common.c op-id-name.c op-class.c emu-mem-io.c emu-proc.c emu-serial.c emu-int.c op-exec.c emu-main.c
EMU86_OBJS = op-common.o op-id-name.o op-class.o emu-mem-io.o emu-proc.o emu-serial.o emu-int.o op-exec.o emu-main.o

# PCAT utility for EMU86 serial stub

PCAT_PROG = pcat

PCAT_SRCS = pcat-main.c
PCAT_OBJS = pcat-main.o

# Rules

.PHONY: all install clean

all: $(EMU86_PROG) $(PCAT_PROG)

install: $(EMU86_PROG) $(PCAT_PROG)
	install -m 755 -s $(EMU86_PROG) $(PREFIX)/bin/$(EMU86_PROG)
	install -m 755 -s $(PCAT_PROG) $(PREFIX)/bin/$(PCAT_PROG)

clean:	
	rm -f $(EMU86_OBJS) $(EMU86_PROG) $(PCAT_OBJS) $(PCAT_PROG)

$(EMU86_OBJS): $(EMU86_HDRS)

$(EMU86_PROG): $(EMU86_OBJS)
	$(CC) -o $(EMU86_PROG) $(EMU86_OBJS)

$(PCAT_PROG): $(PCAT_OBJS)
	$(CC) -o $(PCAT_PROG) $(PCAT_OBJS)
