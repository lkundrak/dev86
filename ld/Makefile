
LIBDIR	=/usr/bin
CFLAGS	=-O
LDFLAGS	=-s

# May need some of these if the auto-sense fails.
# -DV7_A_OUT -DBSD_A_OUT -DSTANDARD_GNU_A_OUT
DEFS	=-DREL_OUTPUT

OBJS= dumps.o io.o ld.o readobj.o table.o typeconv.o linksyms.o \
      writex86.o writebin.o

all: ld86

ld86: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

install: ld86
	install -d $(LIBDIR)
	install -m 755 ld86 $(LIBDIR)

clean realclean:
	rm -f $(OBJS) ld86

$(OBJS): align.h ar.h bindef.h byteord.h config.h const.h globvar.h obj.h \
	 syshead.h type.h x86_aout.h

ar.h:
	ln -s ../libc/include/ar.h . || \
	ln ../libc/include/ar.h .

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@
