# Makefile for host stubs and tools

.PHONY : all

all : \
  pt1-stub pt1-read pt1-write pt1-exec \
  mon86-stub mon86-host

CFLAGS = -g -D HOST_STUB

# First stage monitor

pt1-stub.o : pt1-target.c
	$(CC) $(CFLAGS) -o $@ -c $<

pt1-stub : pt1-stub.o

# Second stage monitor

mon86-common-host.o : mon86-common.c
	$(CC) $(CFLAGS) -o $@ -c $<

mon86-stub.o : mon86-target.c
	$(CC) $(CFLAGS) -o $@ -c $<

mon86-stub : mon86-common-host.o mon86-stub.o

mon86-host : mon86-common-host.o mon86-host.o
