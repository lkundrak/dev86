#!/bin/sh

echo 'rm -f *.o test *.a'
rm -f *.o test *.a
echo 'make'
make
echo 'ar r libnat.a *.o'
ar r libnat.a *.o
echo 'ranlib libnat.a'
ranlib libnat.a
echo 'gcc test.c -I- -I../include -fno-builtin -o test.o -L./ -lnat'
gcc test.c -I- -I../include -fno-builtin -o test.o -L./ -lnat
