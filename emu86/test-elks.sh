#!/bin/bash

# Run ELKS minimal configuration (see config-elks)

./emu86 -w 0xe0000 -f ../../elks/elks/arch/i86/boot/Image -w 0x80000 -f ../../elks/elks/tools/romfs.bin -x 0xe000:0x42 &

sleep 1

# Listen to serial port

./pcat `cat emu86.pts`
