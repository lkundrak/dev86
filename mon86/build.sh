#/bin/sh

# Build stub for host

gcc -g -D HOST_STUB -o pt1-stub pt1-target-main.c

# Build for target

bcc -ansi -0 -c pt1-target-main.c
ld86 -0 -d -o pt1-target pt1-target-main.o
