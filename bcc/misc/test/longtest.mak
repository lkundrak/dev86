cc -o cclongtest longtest.c
sc longtest.c sclongtest.s
cc -o sclongtest sclongtest.s $HOME/lib/libcb.a $HOME/lib/liblb.a
