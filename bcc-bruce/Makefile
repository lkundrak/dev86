#
#	Build bcc
#

all:		bcc bcc-cc1

CFLAGS = -DLOCALPREFIX="\"/usr/local\"" -O2

SRCS		=assign.c declare.c gencode.c label.c preserve.c type.c \
		 bcc-cc1.c express.c genloads.c loadexp.c scan.c \
		 exptree.c glogcode.c longop.c softop.c codefrag.c \
		 floatop.c hardop.c output.c state.c debug.c function.c \
		 input.c preproc.c table.c

OBJS		=assign.o declare.o gencode.o label.o preserve.o type.o \
		 bcc-cc1.o express.o genloads.o loadexp.o scan.o \
		 exptree.o glogcode.o longop.o softop.o codefrag.o \
		 floatop.o hardop.o output.o state.o debug.o function.o \
		 input.o preproc.o table.o


bcc:		bcc.c
		$(CC) $(CFLAGS) -o bcc bcc.c

bcc-cc1:	$(OBJS)
		$(CC) $(CFLAGS) -o bcc-cc1 $(OBJS)

