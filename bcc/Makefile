#
# bcc Makefile
# $Id$
#

PREFIX=/usr

CFLAGS	=-O
LDFLAGS	=-s
BINDIR	=$(PREFIX)/bin
LIBDIR	=$(PREFIX)/lib/bcc
BCCDEFS	=-DLOCALPREFIX=$(PREFIX) -DBINDIR=$(BINDIR) -DDEFARCH=0

BCFLAGS=$(ANSI) $(CFLAGS) $(LDFLAGS)

OBJS = bcc-cc1.o codefrag.o dbnode.o declare.o express.o exptree.o floatop.o \
	function.o gencode.o genloads.o glogcode.o hardop.o input.o label.o \
	loadexp.o longop.o output.o preproc.o preserve.o scan.o softop.o \
	state.o table.o type.o assign.o hashcmd.o debug.o dbprintf.o

all:	bcc-cc1 bcc 

install: all
	install -d $(BINDIR)
	install -m 755 bcc $(BINDIR)/bcc
	install -d $(LIBDIR)
	install -m 755 bcc-cc1 $(LIBDIR)/bcc-cc1

bcc: bcc.c
	$(CC) $(BCFLAGS) $(BCCDEFS) bcc.c -o $@

ncc: bcc.c
	$(CC) $(BCFLAGS) -DLOCALPREFIX= -DDEFARCH=0 bcc.c -o $@

bcc09: bcc.c
	$(CC) $(BCFLAGS) -DMC6809 $(BCCDEFS) bcc.c -o $@

ccc: bcc.c
	$(CC) $(BCFLAGS) -DCCC $(BCCDEFS) bcc.c -o $@

bcc-cc1: $(OBJS)
	$(CC) $(BCCARCH) $(LDFLAGS) $(OBJS) -o bcc-cc1

debug.o: debug.c debug.h
	$(CC) $(ANSI) $(BCCARCH) $(CFLAGS) -c debug.c

dbprintf.o: dbprintf.c
	$(CC) $(ANSI) $(BCCARCH) $(CFLAGS) -c dbprintf.c

clean realclean:
	rm -f bcc bcc-cc1 ncc bcc09 ccc bcc.o $(OBJS)


$(OBJS):  bcc.h align.h const.h types.h \
          byteord.h condcode.h gencode.h \
	  input.h label.h os.h output.h \
	  parse.h proto.h reg.h sc.h scan.h \
	  sizes.h table.h type.h

.c.o:
	$(CC) $(BCCARCH) $(CFLAGS) -c $<
