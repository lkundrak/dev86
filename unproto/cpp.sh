#!/bin/sh

# @(#) cpp.sh 1.2 91/09/22 21:21:43

# Unprototypeing preprocessor for non-ANSI C compilers. Define __STDC__
# if you have enough courage. You will have to modify this script if
# your cc(1) command specifies output file names to the preprocessor.

exec /lib/cpp "$@" -Dconst= -Dvolatile= | unproto 
