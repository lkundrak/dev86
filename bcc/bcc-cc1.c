/* bcc-cc1.c - "pass 1" for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"

PUBLIC int main(argc, argv)
int argc;
char **argv;
{
    debug(1, "Start");
    growheap(0);		/* init order is important */
    syminit();
    etreeinit();
#ifdef BUILTIN_CPP
    ifinit();
    predefine();
#endif
    openio(argc, argv);
    codeinit();
    typeinit();
    program();
    finishup();

    /* NOTREACHED */
    return 0;
}
