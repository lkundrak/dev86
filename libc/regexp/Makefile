
OBJ=regexp.o regsub.o
LSRC=regexp.c regsub.c regerror.c

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

all: $(LIBC)
	@$(RM) $(OBJ)

$(LIBC): $(LIBC)($(OBJ))

transfer:
	-@rm -f ../include/regexp.h ../include/regmagic.h
	cp -p regexp.h regmagic.h ../include/.

$(LIBC)(regexp.o):	regexp.c regexp.h regmagic.h
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

$(LIBC)(regsub.o):	regsub.c regexp.h regmagic.h
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(AR) $(ARFLAGS) $@ $*.o

clean:
	rm -f libc.a *.o core mon.out timer.t.h dMakefile dtr try timer
	rm -f ../include/regexp.h ../include/regmagic.h
