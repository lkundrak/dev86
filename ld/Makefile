CFLAGS		=-O -DBSD_A_OUT -DSTANDARD_GNU_A_OUT
LDFLAGS		=-N -s

OBJS		=dumps.o io.o ld.o readobj.o table.o typeconv.o writebin.o

ld: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) ld

dumps.o: dumps.c const.h config.h obj.h type.h globvar.h
io.o: io.c const.h config.h obj.h type.h globvar.h
ld.o: ld.c const.h config.h byteord.h type.h globvar.h
readobj.o: readobj.c const.h config.h byteord.h obj.h type.h globvar.h
table.o: table.c const.h config.h align.h obj.h type.h globvar.h
typeconv.o: typeconv.c const.h config.h type.h globvar.h
writebin.o: writebin.c const.h config.h obj.h type.h globvar.h
