#!/bin/bash

# Build binaries for target

# First stage monitor

bcc -ansi -0 -c pt1-target.c
ld86 -0 -d -M -o pt1.bin pt1-target.o > pt1.map

# Second stage monitor

bcc -ansi -0 -c -o mon86-common-target.o mon86-common.c
bcc -ansi -0 -c -o mon86-target.o mon86-target.c
as86 -0 -o mon86-low.o mon86-low.s
ld86 -0 -d -M -o mon86.bin mon86-low.o mon86-common-target.o mon86-target.o > mon86.map
