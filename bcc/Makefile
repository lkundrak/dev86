#
# bcc Makefile
# $Id$
#

PREFIX=/usr

CFLAGS	=-O
LDFLAGS	=-s
BINDIR	=/usr/bin
LIBDIR	=/usr/lib/bcc
BCCDEFS	=-DLOCALPREFIX=$(PREFIX) -DDEFARCH=0

OBJS = bcc-cc1.o codefrag.o debug.o declare.o express.o exptree.o floatop.o \
	function.o gencode.o genloads.o glogcode.o hardop.o input.o label.o \
	loadexp.o longop.o output.o preproc.o preserve.o scan.o softop.o \
	state.o table.o type.o assign.o	

all:	bcc-cc1 bcc 

install: all
	install -d $(BINDIR)
	install -m 755 bcc $(BINDIR)/bcc
	install -d $(LIBDIR)
	install -m 755 bcc-cc1 $(LIBDIR)/bcc-cc1

bcc: bcc.c
	$(CC) -ansi $(CFLAGS) $(BCCDEFS) $(LDFLAGS) bcc.c -o $@

ncc: bcc.c
	$(CC) -ansi $(CFLAGS) -DL_TREE -DDEFARCH=0 $(LDFLAGS) bcc.c -o $@

bcc09: bcc.c
	$(CC) -ansi $(CFLAGS) -DMC6809 $(BCCDEFS) $(LDFLAGS) bcc.c -o $@

ccc: bcc.c
	$(CC) -ansi $(CFLAGS) -DCCC $(BCCDEFS) $(LDFLAGS) bcc.c -o $@

bcc-cc1: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o bcc-cc1

clean realclean:
	rm -f bcc bcc-cc1 ncc bcc09 ccc bcc.o $(OBJS)


$(OBJS): align.h byteord.h condcode.h const.h gencode.h input.h label.h os.h \
   output.h parse.h proto.h reg.h sc.h scan.h sizes.h table.h type.h types.h
