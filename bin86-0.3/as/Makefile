CC		=gcc
CFLAGS		=-O1 -fomit-frame-pointer -Dwarn=as_warn
INC_CFLAGS	=-I../a.out.h
LDFLAGS		=-s

OBJS		=as.o assemble.o error.o express.o \
		 genbin.o genlist.o genobj.o gensym.o \
		 keywords.o macro.o mops.o pops.o \
		 readsrc.o scan.o table.o ../ld/typeconv.o

.SUFFIXES:
.SUFFIXES: .c .o 

.c.o:
	$(CC) $(CFLAGS) $(INC_CFLAGS) -c $<

all: as86

as86: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) 

clean:
	$(RM) *.o as86 core endian.h

as.o: const.h type.h byteord.h macro.h file.h flag.h globvar.h
assemble.o: const.h type.h address.h globvar.h opcode.h scan.h
error.o: const.h type.h
express.o: const.h type.h address.h globvar.h scan.h source.h
genbin.o: const.h type.h address.h file.h globvar.h
genlist.o: const.h type.h address.h flag.h file.h globvar.h macro.h scan.h \
	   source.h
genobj.o: const.h type.h address.h file.h globvar.h
gensym.o: const.h type.h flag.h file.h globvar.h
keywords.o: const.h type.h opcode.h
macro.o: const.h type.h globvar.h scan.h macro.h
mops.o: const.h type.h globvar.h opcode.h scan.h address.h
pops.o: const.h type.h address.h flag.h globvar.h opcode.h scan.h
readsrc.o: const.h type.h flag.h file.h globvar.h macro.h scan.h source.h
scan.o: const.h type.h scan.h
table.o: const.h type.h globvar.h scan.h

const.h: endian.h

endian.h: det_endian.c
	$(CC) -o det_endian det_endian.c
	-@if [ $$? = 0 ]; then \
	  det_endian > $@; \
	  if [ $$? = 0 ]; then \
	    rm -f det_endian; \
	  else \
	    echo Failed to create $@; \
	    exit 1; \
	  fi; \
	else \
	  echo Failed to compile det_endian.c; \
	  exit 1; \
	fi
