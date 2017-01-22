# Makefile for host stubs and tools

.PHONY : all

all : \
  mon86-stub \
  mon86-host \
  # end of list

CFLAGS = -g -D HOST_STUB

# Second stage monitor

mon86-common-host.o : mon86-common.c
	$(CC) $(CFLAGS) -o $@ -c $<

mon86-stub.o : mon86-target.c
	$(CC) $(CFLAGS) -o $@ -c $<

mon86-stub : mon86-common-host.o mon86-stub.o

mon86-host : mon86-common-host.o mon86-host.o
