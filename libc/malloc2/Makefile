
TOP=..
include $(TOP)/Make.defs

MOBJ=malloc.o stack.o

OBJ=$(MOBJ)

all: $(OBJ)

libc.a: $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

clean:
	rm -f $(OBJ) libc.a

$(AOBJ): $(ASRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(ASRC)

transfer:
	-@rm ../include/malloc.h
	cp -p malloc.h ../include/.
