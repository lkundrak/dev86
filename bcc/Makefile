#
# bcc Makefile
# $Id$
#
BCCDEFS=-DLOCALPREFIX="\"/usr\"" -DDEFARCH=0

ifneq ($(TOPDIR),)
include $(TOPDIR)/Make.defs
BCCDEFS=-DLOCALPREFIX="\"$(BCCHOME)\"" -DDEFARCH=0
else
CC=bcc
CFLAGS=-Mf
LDFLAGS=-s -M
endif

OBJS = bcc-cc1.o codefrag.o debug.o declare.o express.o exptree.o floatop.o \
	function.o gencode.o genloads.o glogcode.o hardop.o input.o label.o \
	loadexp.o longop.o output.o preproc.o preserve.o scan.o softop.o \
	state.o table.o type.o assign.o	

all:	bcc bcc-cc1

install: all
	install -d $(LIBDIR)
	install -m 755 bcc $(BINDIR)/bcc
	install -m 755 bcc-cc1 $(LIBDIR)/bcc-cc1

bcc: bcc.o
	$(CC) -ansi $(LDFLAGS) bcc.o -o $@

bcc.o: bcc.c
	$(CC) -ansi $(CFLAGS) $(BCCDEFS) -c bcc.c

bcc09: bcc.o
	$(CC) -DMC6809 bcc.o $(LDFLAGS) -o $@

ccc: bcc.o
	$(CC) -DCCC bcc.o $(LDFLAGS) -o $@

bcc-cc1: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o bcc-cc1

clean:
	rm -f bcc bcc-cc1 bcc09 ccc bcc.o $(OBJS)
