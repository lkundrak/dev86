
TOP=..
include $(TOP)/Make.defs

ASRC=malloc.c
AOBJ=malloc.o alloca.o free.o calloc.o realloc.o

OBJ=$(AOBJ)

all: $(OBJ)

libc.a: $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

clean:
	rm -f *.o libc.a

$(AOBJ): $(ASRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(ASRC)

transfer:
	-@rm ../include/malloc.h
	cp -p malloc.h ../include/.
