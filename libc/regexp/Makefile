
TOP=..
include $(TOP)/Make.defs

OBJ=regexp.o regsub.o
LSRC=regexp.c regsub.c regerror.c

try:	try.o $(OBJ)
	$(CC) $(CFLAGS) try.o $(OBJ) -o try

# Regression test.
test:	try tests
	@echo 'No news is good news...'
	try <tests

libc.a: $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

transfer:
	-@rm -f ../include/regexp.h ../include/regmagic.h
	cp -p regexp.h regmagic.h ../include/.

regexp.o:	regexp.c regexp.h regmagic.h
regsub.o:	regsub.c regexp.h regmagic.h

clean:
	rm -f libc.a *.o core mon.out timer.t.h dMakefile dtr try timer
