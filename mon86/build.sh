#!/bin/bash

# Build binaries for target

# Second stage monitor

bcc -ansi -0 -c -o mon86-common-target.o mon86-common.c
bcc -ansi -0 -c -o mon86-target.o mon86-target.c

as86 -0 -o mon86-low.o mon86-low.s
as86 -0 -o mon86-arch.o mon86-arch.s

ld86 -0 -d -M -o mon86.bin mon86-low.o mon86-arch.o mon86-common-target.o mon86-target.o > mon86.map
