
TOP=..
include $(TOP)/Make.defs

ASRC=stdio.c
AOBJ=_stdio_init.o fputc.o fgetc.o fflush.o fgets.o gets.o fputs.o	\
     puts.o fread.o fwrite.o fopen.o fclose.o fseek.o rewind.o ftell.o	\
     setbuffer.o setvbuf.o ungetc.o

PSRC=printf.c
POBJ=printf.o sprintf.o fprintf.o vprintf.o vsprintf.o vfprintf.o

SSRC=scanf.c
SOBJ=scanf.o sscanf.o fscanf.o vscanf.o vsscanf.o vfscanf.o

OBJ= $(AOBJ) $(POBJ) $(SOBJ)

all: $(OBJ)

libc.a: $(OBJ)
	ar r ../$(LIBC) $(OBJ)
	@touch libc.a

transfer:
	-@rm -f ../include/stdio.h
	cp -p stdio.h ../include/.

clean:
	rm -f *.o libc.a

$(AOBJ): $(ASRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(ASRC)

$(POBJ): $(PSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(PSRC)

$(SOBJ): $(SSRC)
	$(CC) $(CFLAGS) -c -DL_$* -o $@ $(SSRC)

