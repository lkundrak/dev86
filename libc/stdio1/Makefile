TOP=..
include $(TOP)/Make.defs
OBJ = __ffillbuf.o __stdio_init.o fflush.o fgetc.o fgets.o    \
	fputc.o fputs.o puts.o printf.o fopen.o fclose.o scanf.o

#missing functions from above: printf etc, scanf etc, idealgetline
#seek

#fopen.c, fclose.c removed because of malloc() use

CFLAGS+=$(WALL)

ifeq ($(PLATFORM),i86-ELKS)
CFLAGS=$(CCFLAGS) $(DEFS) -ansi
endif

all: $(OBJ)

libc.a: $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

$(OBJ): stdio.h

test: test.o $(OBJ)
	$(CC) -o test test.o $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $< -c

clean:
	rm -f *.o test libc.a

transfer:
	-@rm -f ../include/stdio.h
	cp -p stdio.h ../include/.
