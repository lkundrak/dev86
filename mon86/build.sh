#/bin/sh

# Build stub for host

gcc -g -D HOST_STUB -o pt1-stub pt1-target-main.c
gcc -g -D HOST_STUB -o mon86-stub mon86-target-main.c

# Build binaries for target

bcc -ansi -0 -c pt1-target-main.c
ld86 -0 -d -o pt1-target pt1-target-main.o

bcc -ansi -0 -c mon86-target-main.c
ld86 -0 -d -o mon86-target mon86-target-main.o

# Build tools for host

gcc -g -o pt1-read pt1-read.c
gcc -g -o pt1-write pt1-write.c
gcc -g -o pt1-exec pt1-exec.c
