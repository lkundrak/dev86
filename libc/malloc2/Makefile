
OBJ=malloc.o stack.o

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

all: $(LIBC)($(OBJ))
	@:
clean:
	rm -f *.o libc.a

transfer:
	-@rm ../include/malloc.h
	cp -p malloc.h ../include/.
