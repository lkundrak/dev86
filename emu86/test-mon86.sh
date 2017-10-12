#!/bin/bash

# Start to execute MON86 @ F000h:0

./emu86 -w 0xf0000 -f ../mon86/mon86.bin -x 0xf000:0x0 &

# Give enough time to emulator to open PTY

sleep 1

# Send input reference to PTY

./pcat -t 1000 `cat emu86.pts` <../mon86/test-mon86-in.txt >test-mon86-out.txt

# Compare with output reference

diff ../mon86/test-mon86-ref.txt test-mon86-out.txt
if (( $? > 0 )); then
	echo "FAILED"
	else
	echo "PASSED"
	fi