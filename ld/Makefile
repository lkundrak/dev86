
ifneq ($(TOPDIR),)
include $(TOPDIR)/Make.defs
else
CC=bcc
LDFLAGS=-s
endif

OBJS		=dumps.o io.o ld.o readobj.o table.o typeconv.o writebin.o

all: ld86

ld86: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

install: ld86
	install -d $(LIBDIR)
	install -m 755 ld86 $(LIBDIR)

clean:
	rm -f $(OBJS) ld86

dumps.o: dumps.c const.h config.h obj.h type.h globvar.h
io.o: io.c const.h config.h obj.h type.h globvar.h
ld.o: ld.c const.h config.h byteord.h type.h globvar.h
readobj.o: readobj.c const.h config.h byteord.h obj.h type.h globvar.h
table.o: table.c const.h config.h align.h obj.h type.h globvar.h
typeconv.o: typeconv.c const.h config.h type.h globvar.h
writebin.o: writebin.c const.h config.h obj.h type.h globvar.h
