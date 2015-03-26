#/bin/sh

# Build stub for host

gcc -D HOST_STUB -o pt1-stub pt1-target-main.c

#  Test stub on host

./pt1-stub <test-in.txt >test-out.txt
diff test-ref.txt test-out.txt

# Build for target

bcc -ansi -0 -c pt1-target-main.c
#as86 -0 -o pt1-target-start.o pt1-target-start.s
#ld86 -0 -o pt1-target pt1-target-start.o pt1-target-main.o
ld86 -0 -d -o pt1-target pt1-target-main.o
