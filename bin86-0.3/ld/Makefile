CC              =gcc
CFLAGS          =-O1 -fomit-frame-pointer 
INC_CFLAGS	=-DBSD_A_OUT -I../a.out.h -D__linux__
LDFLAGS		=-s 

OBJS		=dumps.o io.o ld.o readobj.o table.o typeconv.o \
		writebin.o

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INC_CFLAGS) -c $<

all: ld86

ld86: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	$(RM) $(OBJS) ld86 core

dumps.o: dumps.c const.h config.h obj.h type.h globvar.h
io.o: io.c const.h config.h type.h globvar.h
ld.o: ld.c const.h config.h type.h byteord.h globvar.h
readobj.o: readobj.c const.h config.h byteord.h obj.h type.h globvar.h
table.o: table.c const.h config.h align.h obj.h type.h globvar.h
typeconv.o: typeconv.c const.h config.h type.h globvar.h
writebin.o: writebin.c const.h config.h obj.h type.h globvar.h
