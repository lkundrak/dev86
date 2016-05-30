#/bin/sh

# Build stub for host

gcc -g -D HOST_STUB -o pt1-stub pt1-target-main.c
gcc -g -D HOST_STUB -o mon86-stub mon86-common.c mon86-main.c

# Build binaries for target

bcc -ansi -0 -c pt1-target-main.c
ld86 -0 -d -M -o pt1-target pt1-target-main.o > pt1-target.map

bcc -ansi -0 -c -o mon86-common.o mon86-common.c
bcc -ansi -0 -c -o mon86-main.o mon86-main.c
as86 -0 -o mon86-low.o mon86-low.s
ld86 -0 -d -M -o mon86.bin mon86-low.o mon86-common.o mon86-main.o > mon86.map

# Build tools for host

gcc -g -o pt1-read pt1-read.c
gcc -g -o pt1-write pt1-write.c
gcc -g -o pt1-exec pt1-exec.c
