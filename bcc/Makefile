#
# bcc Makefile
# $Id$
#

PREFIX=/usr
LIBPRE=$(PREFIX)

CFLAGS	=-O
LDFLAGS	=-s
BINDIR	=$(PREFIX)/bin
LIBDIR	=$(LIBPRE)/lib/bcc
BCCDEFS	=-DLOCALPREFIX=$(LIBPRE) -DBINDIR=$(BINDIR) -DDEFARCH=0

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
	$(CC) $(ANSI) $(CFLAGS) $(BCCDEFS) $(LDFLAGS) $^ -o $@

ncc: bcc.c
	$(CC) $(ANSI) $(CFLAGS) -DL_TREE -DDEFARCH=0 $(LDFLAGS) $^ -o $@

bcc09: bcc.c
	$(CC) $(ANSI) $(CFLAGS) -DMC6809 $(BCCDEFS) $(LDFLAGS) $^ -o $@

ccc: bcc.c
	$(CC) $(ANSI) $(CFLAGS) -DCCC $(BCCDEFS) $(LDFLAGS) $^ -o $@

bcc-cc1: $(OBJS)
	$(CC) $(BCCARCH) $(LDFLAGS) $(OBJS) -o bcc-cc1

clean realclean:
	rm -f bcc bcc-cc1 ncc bcc09 ccc bcc.o $(OBJS)


$(OBJS):  bcc.h align.h const.h types.h \
          byteord.h condcode.h gencode.h \
	  input.h label.h os.h output.h \
	  parse.h proto.h reg.h sc.h scan.h \
	  sizes.h table.h type.h

.c.o:
	$(CC) $(BCCARCH) $(CFLAGS) -c $<
